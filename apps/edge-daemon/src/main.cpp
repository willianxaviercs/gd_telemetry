#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "core/redis_stream_publisher.h"
#include "core/sqlite_device_store.h"

#include "Config.h"
#include "EventPublisher.h"

int main(int argc, char** argv)
{
    Config config = {};
    if (!ParseArgs(argc, argv, config))
    {
        Usage(argv[0]);
        return 1;
    }

    if (!std::filesystem::exists(config.db_path))
    {
        std::cerr << "missing sqlite database: " << config.db_path << std::endl;
        return 1;
    }

    std::cout << "edge-daemon: db_path=" << config.db_path << std::endl;
    std::cout << "edge-daemon: redis=" << config.redis_host
            << ":" << config.redis_port
            << " stream=" << config.redis_stream << std::endl;

    SqliteDeviceStore store(config.db_path);
    RedisStreamPublisher redis(config.redis_host, config.redis_port);

    EventPublisher event_publisher(config, store, redis);
    event_publisher.Run();

    return 0;
}
