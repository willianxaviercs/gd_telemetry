#include "Config.h"
#include "StreamConsumer.h"

#include <iostream>
#include <format>

using namespace proto::v1;

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
                    // TODO(wxr): later we can use stream id for idempotent insert??
                    std::cout << "processing id: " << entry.first << std::endl;

                    const auto& fields = entry.second;

                    if (fields.empty())
                        continue;
                    
                    // TODO(wxr): Can we use type data other than string??
                    // example:
                    // fields [{"device_id", "1"}, {"timestamp", "172731231230"}, {"type", "1"}, {"payload", "0xffafbgdc..."}] 

                    const auto& device_id = fields[0].second;
                    const auto& timestamp = fields[1].second;
                    const auto& type      = fields[2].second;
                    const auto& blob      = fields[3].second;

                    Payload p;
                    p.ParseFromString(blob);

                    std::cout << p.DebugString() << std::endl;
                    storage_.InsertEvent(p, device_id, timestamp, type);

                    redis_.xack(config_.stream, config_.group, entry.first);
                }
            }
        }
        catch (const std::exception& e)
        {
            ;
            //std::cerr << "error: " << e.what() << std::endl;
        }
    }
}

