#include "Config.h"
#include "StreamConsumer.h"

#include <stdexcept>
#include <iostream>
#include <format>

using namespace proto::v1;

namespace
{
const std::string& RequireField(const Fields& fields, const std::string& name)
{
    for (const auto& [field_name, field_value] : fields)
    {
        if (field_name == name)
            return field_value;
    }

    throw std::runtime_error(std::format("missing stream field: {}", name));
}
}

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

                    const auto& device_id = RequireField(fields, "device_id");
                    const auto& timestamp = RequireField(fields, "timestamp");
                    const auto& type      = RequireField(fields, "type");
                    const auto& blob      = RequireField(fields, "payload");

                    Payload p;
                    if (!p.ParseFromString(blob))
                        throw std::runtime_error(std::format(
                            "invalid protobuf payload for stream entry {}",
                            entry.first));

                    std::cout << p.DebugString() << std::endl;
                    storage_.InsertEvent(p, device_id, timestamp, type);

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
