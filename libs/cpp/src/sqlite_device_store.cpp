#include "core/sqlite_device_store.h"
#include "core/device_event.h"

#include <sqlite3.h>

#include <sstream>
#include <stdexcept>

void CheckSqlite(int rc, sqlite3* db, const char* context)
{
    if (rc == SQLITE_OK || rc == SQLITE_DONE || rc == SQLITE_ROW)
        return;

    std::ostringstream message;
    message << context << ": " << sqlite3_errmsg(db);
    throw std::runtime_error(message.str());
}

class StatementFinalizer
{
    sqlite3_stmt* stmt_;
  public:
    explicit StatementFinalizer(sqlite3_stmt* stmt)
        : stmt_(stmt)
    {}

    ~StatementFinalizer()
    {
        if (stmt_ != nullptr)
            sqlite3_finalize(stmt_);
    }
};

SqliteDeviceStore::SqliteDeviceStore(const std::filesystem::path& db_path)
    : db_(nullptr)
{
    if (sqlite3_open(db_path.string().c_str(), &db_) != SQLITE_OK)
        throw std::runtime_error("open sqlite database: " + db_path.string());

    try
    {
        CheckSqlite(sqlite3_busy_timeout(db_, 1000), db_, "set busy timeout");
        CheckSqlite(
            sqlite3_exec(db_, "PRAGMA journal_mode = WAL", nullptr, nullptr, nullptr),
            db_,
            "enable WAL mode");
    }
    catch (...)
    {
        sqlite3_close(db_);
        db_ = nullptr;
        throw;
    }
}

SqliteDeviceStore::~SqliteDeviceStore()
{
    if (db_ != nullptr)
        sqlite3_close(db_);
}

std::int64_t SqliteDeviceStore::ReadLastPublishedId(void)
{
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT last_published_id FROM collector_state WHERE id = 1";

    CheckSqlite(sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr), db_, "prepare state query");
    StatementFinalizer stmt_finalizer(stmt);

    if (sqlite3_step(stmt) != SQLITE_ROW)
        throw std::runtime_error("collector_state row is missing");

    return sqlite3_column_int64(stmt, 0);
}

// TODO(wxr): Change this to ReadEvents (batching processing)
std::optional<DeviceEvent> SqliteDeviceStore::ReadNextEvent(std::int64_t last_published_id)
{
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT id, device_id, timestamp, type, payload "
        "FROM device_events "
        "WHERE id > ? "
        "ORDER BY id ASC "
        "LIMIT 1";

    CheckSqlite(sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr), db_, "prepare event query");
    StatementFinalizer stmt_finalizer(stmt);

    CheckSqlite(sqlite3_bind_int64(stmt, 1, last_published_id), db_, "bind event query");

    const int rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE)
        return std::nullopt;

    CheckSqlite(rc, db_, "step event query");

    DeviceEvent event = {};
    event.id        = sqlite3_column_int64(stmt, 0);
    event.device_id = sqlite3_column_int64(stmt, 1);
    event.timestamp = sqlite3_column_int64(stmt, 2);
    event.type      = static_cast<EventType>(sqlite3_column_int(stmt, 3));

    if (sqlite3_column_type(stmt, 4) != SQLITE_BLOB)
        throw std::runtime_error("device_events.payload must be a BLOB");

    const char* blob_data = static_cast<const char*>(sqlite3_column_blob(stmt, 4));
    const int   blob_size = sqlite3_column_bytes(stmt, 4);
    if (blob_data == nullptr || blob_size <= 0)
        throw std::runtime_error("device_events.event_blob is empty");

    event.payload.assign(blob_data, static_cast<std::size_t>(blob_size));

    return event;
}

void SqliteDeviceStore::UpdateLastPublishedId(std::int64_t last_published_id)
{
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE collector_state SET last_published_id = ? WHERE id = 1";

    CheckSqlite(sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr), db_, "prepare state update");
    StatementFinalizer stmt_finalizer(stmt);

    CheckSqlite(sqlite3_bind_int64(stmt, 1, last_published_id), db_, "bind state update");
    CheckSqlite(sqlite3_step(stmt), db_, "step state update");
}

