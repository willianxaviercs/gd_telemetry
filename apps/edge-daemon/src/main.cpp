#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "core/device_event.h"
#include "core/redis_stream_publisher.h"
#include "core/sqlite_device_store.h"

struct Config
{
    double poll_interval_seconds;
    std::filesystem::path db_path;
    std::string redis_host;
    int redis_port;
    std::string redis_stream;
};

void usage(const char* program_name)
{
    std::cerr << "usage: " << program_name
              << " --device-db-path <path>"
              << " --poll-interval-seconds <seconds>"
              << " --redis-stream <name>"
              << " [--redis-host <host>]"
              << " [--redis-port <port>]" << std::endl;

    std::exit(1);
}

Config parse_args(int argc, char** argv)
{
    Config config{};
    config.redis_host = "127.0.0.1";
    config.redis_port = 6379;
    bool has_db_path = false;
    bool has_poll_interval = false;
    bool has_redis_stream = false;

    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        if (arg == "--device-db-path")
        {
            if (i + 1 >= argc)
                usage(argv[0]);

            config.db_path = argv[++i];
            has_db_path = true;
            continue;
        }

        if (arg == "--poll-interval-seconds")
        {
            if (i + 1 >= argc)
                usage(argv[0]);

            config.poll_interval_seconds = std::stod(argv[++i]);
            has_poll_interval = true;
            continue;
        }

        if (arg == "--redis-stream")
        {
            if (i + 1 >= argc)
                usage(argv[0]);

            config.redis_stream = argv[++i];
            has_redis_stream = true;
            continue;
        }

        if (arg == "--redis-host")
        {
            if (i + 1 >= argc)
                usage(argv[0]);

            config.redis_host = argv[++i];
            continue;
        }

        if (arg == "--redis-port")
        {
            if (i + 1 >= argc)
                usage(argv[0]);

            config.redis_port = std::stoi(argv[++i]);
            continue;
        }

        usage(argv[0]);
    }

    if (!has_db_path || !has_poll_interval || !has_redis_stream)
        usage(argv[0]);

    return config;
}

int main(int argc, char** argv)
{
    try
    {
        const Config config = parse_args(argc, argv);

        if (!std::filesystem::exists(config.db_path))
        {
            std::cerr << "missing sqlite database: " << config.db_path << std::endl;
            return 1;
        }

        SqliteDeviceStore store(config.db_path);
        RedisStreamPublisher redis(config.redis_host, config.redis_port);

        std::cout << "edge-daemon: db_path=" << config.db_path << std::endl;
        std::cout << "edge-daemon: redis=" << config.redis_host
                  << ":" << config.redis_port
                  << " stream=" << config.redis_stream << std::endl;

        while (true)
        {
            const long long last_published_id = store.ReadLastPublishedId();
            const auto next_event = store.ReadNextEvent(last_published_id);

            if (!next_event.has_value())
            {
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(config.poll_interval_seconds));
                continue;
            }

            const std::string stream_entry_id = redis.Publish(config.redis_stream, *next_event);
            store.UpdateLastPublishedId(next_event->id);
        }

        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "edge-daemon error: " << error.what() << std::endl;
        return 1;
    }
}
