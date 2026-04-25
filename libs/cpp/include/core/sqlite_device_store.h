#pragma once

#include <filesystem>
#include <optional>

#include "core/device_event.h"

struct sqlite3;

class SqliteDeviceStore
{
public:
    explicit SqliteDeviceStore(const std::filesystem::path& db_path);
    ~SqliteDeviceStore();

    SqliteDeviceStore(const SqliteDeviceStore&) = delete;
    SqliteDeviceStore& operator=(const SqliteDeviceStore&) = delete;

    std::optional<DeviceEvent> ReadNextEvent(std::int64_t last_published_id);
    std::int64_t ReadLastPublishedId();
    void UpdateLastPublishedId(std::int64_t last_published_id);

private:
    sqlite3* db_;
};

