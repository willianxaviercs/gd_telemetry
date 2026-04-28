#pragma once

#include "core/device_event.h"
#include "core/sqlite_device_store.h"
#include "Config.h"

#include <sw/redis++/redis.h>
#include <chrono>
#include <iostream>
#include <thread>

class EventPublisher
{
    static constexpr double RETRY_SECS = 1.0;

    Config&             config_;
    SqliteDeviceStore&  storage_;
    sw::redis::Redis&   redis_;

public:
    EventPublisher(Config& config, SqliteDeviceStore& storage, sw::redis::Redis& redis);
    void Run();
};

