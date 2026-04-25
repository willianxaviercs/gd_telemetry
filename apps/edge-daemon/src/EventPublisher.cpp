#include "EventPublisher.h"

#include <vector>
#include <sw/redis++/redis.h>

namespace {

std::string EventTypeValue(const DeviceEvent& event)
{
    return std::to_string(static_cast<int>(event.type));
}

void PublishDeviceEvent(
    sw::redis::Redis& redis,
    const char*       stream,
    const DeviceEvent& event)
{
    using Field = std::pair<std::string, std::string>;
    std::vector<Field> fields;
    fields.reserve(6);
    fields.emplace_back("event_id", std::to_string(event.id));
    fields.emplace_back("device_id", std::to_string(event.device_id));
    fields.emplace_back("timestamp_unix_ms", std::to_string(event.timestamp));
    fields.emplace_back("type", EventTypeValue(event));

    if (event.type == EventType::TemperatureReading)
    {
        fields.emplace_back("temperature_celsius", std::to_string(event.temperature_celsius));
    }
    else if (event.type == EventType::StatusUpdate)
    {
        fields.emplace_back("status", std::to_string(static_cast<int>(event.device_status)));
    }

    redis.xadd(stream, "*", fields.begin(), fields.end());
}

} // namespace

EventPublisher::EventPublisher(
    Config& config,
    SqliteDeviceStore& storage,
    sw::redis::Redis& redis)
    : config_(config),
      storage_(storage),
      redis_(redis)
{}

void EventPublisher::Run()
{
    for (;;)
    {
        try
        {
            const auto last_published_id = storage_.ReadLastPublishedId();
            const auto next_event = storage_.ReadNextEvent(last_published_id);

            if (!next_event.has_value())
            {
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(config_.poll_interval_seconds));
                continue;
            }

            PublishDeviceEvent(redis_, config_.redis_stream, *next_event);
            storage_.UpdateLastPublishedId(next_event->id);
        }
        catch (const std::exception& err)
        {
            std::cerr << "[ERR] :: EventPublisher :: " << err.what() << std::endl;
            std::cerr << "[INF] :: EventPublisher :: Retrying in "
                      << RETRY_SECS << " seconds" << std::endl;

            std::this_thread::sleep_for(
                std::chrono::duration<double>(RETRY_SECS));
        }
    }
}
