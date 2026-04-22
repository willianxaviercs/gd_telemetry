#include <sqlite3.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr int kEventTypeTemperatureReading = 1;
constexpr int kEventTypeStatusUpdate = 2;

struct Config {
    double poll_interval_seconds;
    std::filesystem::path db_path;
    std::string redis_host;
    int redis_port;
    std::string redis_stream;
};

struct EventRow {
    long long id;
    long long device_id;
    long long timestamp_unix_ms;
    int type;
    bool has_temperature_celsius;
    double temperature_celsius;
    bool has_status;
    int status;
};

[[noreturn]] void usage(const char* program_name) {
    std::cerr << "usage: " << program_name
              << " --device-db-path <path>"
              << " --poll-interval-seconds <seconds>"
              << " --redis-stream <name>"
              << " [--redis-host <host>]"
              << " [--redis-port <port>]" << std::endl;
    std::exit(1);
}

Config parse_args(int argc, char** argv) {
    Config config{};
    config.redis_host = "127.0.0.1";
    config.redis_port = 6379;
    bool has_db_path = false;
    bool has_poll_interval = false;
    bool has_redis_stream = false;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--device-db-path") {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            config.db_path = argv[++i];
            has_db_path = true;
            continue;
        }

        if (arg == "--poll-interval-seconds") {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            config.poll_interval_seconds = std::stod(argv[++i]);
            has_poll_interval = true;
            continue;
        }

        if (arg == "--redis-stream") {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            config.redis_stream = argv[++i];
            has_redis_stream = true;
            continue;
        }

        if (arg == "--redis-host") {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            config.redis_host = argv[++i];
            continue;
        }

        if (arg == "--redis-port") {
            if (i + 1 >= argc) {
                usage(argv[0]);
            }
            config.redis_port = std::stoi(argv[++i]);
            continue;
        }

        usage(argv[0]);
    }

    if (!has_db_path || !has_poll_interval || !has_redis_stream) {
        usage(argv[0]);
    }
    return config;
}

void check_sqlite(int rc, sqlite3* db, const char* context) {
    if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW) {
        return;
    }

    std::ostringstream message;
    message << context << ": " << sqlite3_errmsg(db);
    throw std::runtime_error(message.str());
}

void exec_sql(sqlite3* db, const char* sql, const char* context) {
    char* error_message = nullptr;
    const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &error_message);
    if (rc == SQLITE_OK) {
        return;
    }

    std::ostringstream message;
    message << context << ": ";
    if (error_message != nullptr) {
        message << error_message;
        sqlite3_free(error_message);
    } else {
        message << sqlite3_errmsg(db);
    }

    throw std::runtime_error(message.str());
}

long long read_last_published_id(sqlite3* db) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT last_published_id FROM collector_state WHERE id = 1";

    check_sqlite(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr), db, "prepare state query");

    const int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("collector_state row is missing");
    }

    const long long last_published_id = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);
    return last_published_id;
}

bool read_next_event(sqlite3* db, long long last_published_id, EventRow* row) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT id, device_id, timestamp_unix_ms, type, temperature_celsius, status "
        "FROM device_events "
        "WHERE id > ? "
        "ORDER BY id ASC "
        "LIMIT 1";

    check_sqlite(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr), db, "prepare event query");
    check_sqlite(sqlite3_bind_int64(stmt, 1, last_published_id), db, "bind event query");

    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return false;
    }

    check_sqlite(rc, db, "step event query");

    row->id = sqlite3_column_int64(stmt, 0);
    row->device_id = sqlite3_column_int64(stmt, 1);
    row->timestamp_unix_ms = sqlite3_column_int64(stmt, 2);
    row->type = sqlite3_column_int(stmt, 3);
    row->has_temperature_celsius = sqlite3_column_type(stmt, 4) != SQLITE_NULL;
    row->temperature_celsius = row->has_temperature_celsius ? sqlite3_column_double(stmt, 4) : 0.0;
    row->has_status = sqlite3_column_type(stmt, 5) != SQLITE_NULL;
    row->status = row->has_status ? sqlite3_column_int(stmt, 5) : 0;

    sqlite3_finalize(stmt);
    return true;
}

void update_last_published_id(sqlite3* db, long long last_published_id) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE collector_state SET last_published_id = ? WHERE id = 1";

    check_sqlite(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr), db, "prepare state update");
    check_sqlite(sqlite3_bind_int64(stmt, 1, last_published_id), db, "bind state update");
    check_sqlite(sqlite3_step(stmt), db, "step state update");
    sqlite3_finalize(stmt);
}

const char* status_name(int status) {
    switch (status) {
        case 1:
            return "ONLINE";
        case 2:
            return "OFFLINE";
        case 3:
            return "ERROR";
        default:
            return "STATUS_UNSPECIFIED";
    }
}

std::string format_event(const EventRow& row) {
    std::ostringstream out;
    out << "collector: id=" << row.id
        << " device_id=" << row.device_id
        << " timestamp_unix_ms=" << row.timestamp_unix_ms;

    if (row.type == kEventTypeTemperatureReading && row.has_temperature_celsius) {
        out << " type=TEMPERATURE_READING"
            << " celsius=" << std::fixed << std::setprecision(2) << row.temperature_celsius;
        return out.str();
    }

    if (row.type == kEventTypeStatusUpdate && row.has_status) {
        out << " type=STATUS_UPDATE"
            << " status=" << status_name(row.status);
        return out.str();
    }

    out << " type=UNKNOWN";
    return out.str();
}

class RedisConnection {
  public:
    RedisConnection(const std::string& host, int port) : socket_fd_(-1) {
        struct addrinfo hints {};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        struct addrinfo* result = nullptr;
        const std::string port_string = std::to_string(port);
        const int rc = getaddrinfo(host.c_str(), port_string.c_str(), &hints, &result);
        if (rc != 0) {
            throw std::runtime_error(std::string("resolve redis host: ") + gai_strerror(rc));
        }

        for (struct addrinfo* address = result; address != nullptr; address = address->ai_next) {
            socket_fd_ = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
            if (socket_fd_ < 0) {
                continue;
            }

            if (connect(socket_fd_, address->ai_addr, address->ai_addrlen) == 0) {
                break;
            }

            close(socket_fd_);
            socket_fd_ = -1;
        }

        freeaddrinfo(result);

        if (socket_fd_ < 0) {
            throw std::runtime_error("connect redis");
        }
    }

    ~RedisConnection() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }

    RedisConnection(const RedisConnection&) = delete;
    RedisConnection& operator=(const RedisConnection&) = delete;

    std::string xadd(const std::string& stream_name, const EventRow& row) {
        std::vector<std::string> args{
            "XADD",
            stream_name,
            "*",
            "event_id",
            std::to_string(row.id),
            "device_id",
            std::to_string(row.device_id),
            "timestamp_unix_ms",
            std::to_string(row.timestamp_unix_ms),
            "type",
            std::to_string(row.type),
        };

        if (row.type == kEventTypeTemperatureReading && row.has_temperature_celsius) {
            std::ostringstream value;
            value << std::fixed << std::setprecision(2) << row.temperature_celsius;
            args.emplace_back("temperature_celsius");
            args.emplace_back(value.str());
        } else if (row.type == kEventTypeStatusUpdate && row.has_status) {
            args.emplace_back("status");
            args.emplace_back(std::to_string(row.status));
        }

        write_all(build_resp_array(args));
        return read_bulk_or_simple_string();
    }

  private:
    int socket_fd_;

    static std::string build_resp_array(const std::vector<std::string>& args) {
        std::ostringstream out;
        out << "*" << args.size() << "\r\n";
        for (const std::string& arg : args) {
            out << "$" << arg.size() << "\r\n" << arg << "\r\n";
        }
        return out.str();
    }

    void write_all(const std::string& buffer) {
        size_t total_written = 0;
        while (total_written < buffer.size()) {
            const ssize_t written =
                send(socket_fd_, buffer.data() + total_written, buffer.size() - total_written, 0);
            if (written <= 0) {
                throw std::runtime_error("write redis command");
            }
            total_written += static_cast<size_t>(written);
        }
    }

    std::string read_line() {
        std::string line;
        while (true) {
            char ch = '\0';
            const ssize_t received = recv(socket_fd_, &ch, 1, 0);
            if (received <= 0) {
                throw std::runtime_error("read redis response");
            }

            if (ch == '\r') {
                char lf = '\0';
                const ssize_t lf_received = recv(socket_fd_, &lf, 1, 0);
                if (lf_received <= 0 || lf != '\n') {
                    throw std::runtime_error("read redis response terminator");
                }
                return line;
            }

            line.push_back(ch);
        }
    }

    std::string read_bytes(size_t size) {
        std::string data(size, '\0');
        size_t total_read = 0;
        while (total_read < size) {
            const ssize_t received = recv(socket_fd_, data.data() + total_read, size - total_read, 0);
            if (received <= 0) {
                throw std::runtime_error("read redis payload");
            }
            total_read += static_cast<size_t>(received);
        }
        return data;
    }

    std::string read_bulk_or_simple_string() {
        char prefix = '\0';
        const ssize_t received = recv(socket_fd_, &prefix, 1, 0);
        if (received <= 0) {
            throw std::runtime_error("read redis reply type");
        }

        if (prefix == '+') {
            return read_line();
        }

        if (prefix == '$') {
            const long long length = std::stoll(read_line());
            if (length < 0) {
                throw std::runtime_error("redis returned nil bulk string");
            }

            std::string payload = read_bytes(static_cast<size_t>(length));
            const std::string terminator = read_bytes(2);
            if (terminator != "\r\n") {
                throw std::runtime_error("invalid redis bulk string terminator");
            }
            return payload;
        }

        if (prefix == '-') {
            throw std::runtime_error(std::string("redis error: ") + read_line());
        }

        throw std::runtime_error("unsupported redis reply type");
    }
};

}  // namespace

int main(int argc, char** argv) {
    try {
        const Config config = parse_args(argc, argv);

        if (!std::filesystem::exists(config.db_path)) {
            std::cerr << "missing sqlite database: " << config.db_path << std::endl;
            return 1;
        }

        sqlite3* db = nullptr;
        if (sqlite3_open(config.db_path.string().c_str(), &db) != SQLITE_OK) {
            std::cerr << "failed to open sqlite database: " << config.db_path << std::endl;
            return 1;
        }

        try {
            check_sqlite(sqlite3_busy_timeout(db, 1000), db, "set busy timeout");
            exec_sql(db, "PRAGMA journal_mode = WAL", "enable WAL mode");
            RedisConnection redis(config.redis_host, config.redis_port);

            std::cout << "collector: db_path=" << config.db_path << std::endl;
            std::cout << "collector: redis=" << config.redis_host
                      << ":" << config.redis_port
                      << " stream=" << config.redis_stream << std::endl;

            while (true) {
                const long long last_published_id = read_last_published_id(db);
                EventRow row{};

                if (!read_next_event(db, last_published_id, &row)) {
                    std::this_thread::sleep_for(
                        std::chrono::duration<double>(config.poll_interval_seconds));
                    continue;
                }

                const std::string stream_entry_id = redis.xadd(config.redis_stream, row);
                std::cout << format_event(row)
                          << " redis_stream=" << config.redis_stream
                          << " redis_id=" << stream_entry_id << std::endl;
                update_last_published_id(db, row.id);
            }
        } catch (...) {
            sqlite3_close(db);
            throw;
        }

        sqlite3_close(db);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "collector error: " << error.what() << std::endl;
        return 1;
    }
}
