#include "core/sqlite_device_store.h"
#include <sw/redis++/redis.h>

#include "Config.h"
#include "EventPublisher.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

using namespace sw::redis;

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

    ConnectionOptions redis_opts;
    redis_opts.host = config.redis_host;
    redis_opts.port = config.redis_port;
    Redis redis(redis_opts);

    EventPublisher publisher(config, store, redis);
    publisher.Run();

    return 0;
}

