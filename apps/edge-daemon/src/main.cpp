#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>

#include "config.h"
#include "core/device_event.h"
#include "core/redis_stream_publisher.h"
#include "core/sqlite_device_store.h"

int main(int argc, char** argv)
{
    try
    {
        Config config = {};
        if (!parse_args(argc, argv, config))
        {
            usage(argv[0]);
            return 1;
        }

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
