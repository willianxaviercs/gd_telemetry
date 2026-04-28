#include "Config.h"
#include "StreamConsumer.h"

#include <iostream>
#include <format>

StreamConsumer::StreamConsumer(
        ConsumerConfig& config,
        sw::redis::Redis& redis,
        Storage& storage)
    : 
        config_(config)
      , redis_(redis)
      , storage_(storage)
{}

void StreamConsumer::Run(void) const
{
    // create consumer group if it not exist
    try
    {
        std::cout << std::format("Creating consumer group {} for stream {}",
                    config_.group, config_.stream) << std::endl;

        redis_.xgroup_create(config_.stream, config_.group, "$", true);
    }
    catch (...)
    {
        std::cerr << "Group already exists, ignoring" << std::endl;
    }

    RedisStreamResult result;
    for (;;)
    {
        try
        {
            result.clear();

            redis_.xreadgroup(
                config_.group,
                config_.name,
                config_.stream,
                ">",
                1,
                std::inserter(result, result.end())
            );

            for (const auto& [streamName, entries] : result)
            {
                for (const auto& entry : entries)
                {
                    std::cout << "processing id: " << entry.first << std::endl;

                    const auto& fields = entry.second;

                    if (fields.empty())
                        continue;

                    const auto& payload = fields.front().second;

                    gundam::v1::DeviceEvent msg;
                    msg.ParseFromString(payload);

                    std::cout << msg.DebugString() << std::endl;
                    storage_.InsertEvent(msg);

                    redis_.xack(config_.stream, config_.group, entry.first);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "error: " << e.what() << std::endl;
        }
    }
}

