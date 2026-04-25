#pragma once

#include "core/device_event.h"
#include "core/redis_stream_publisher.h"
#include "core/sqlite_device_store.h"

#include "Config.h"

#include <chrono>
#include <thread>
#include <iostream>

class EventPublisher
{
    static constexpr double RETRY_SECS = 1.0;

    Config& config_;
    SqliteDeviceStore& storage_;
    RedisStreamPublisher& redis_;

public:
    EventPublisher(Config& config, SqliteDeviceStore& storage, RedisStreamPublisher& redis);
    void Run();
};
