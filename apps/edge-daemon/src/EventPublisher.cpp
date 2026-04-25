#include "EventPublisher.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "proto/device_event.pb.h"
#include <sw/redis++/redis.h>

namespace {

struct ParsedDeviceEvent
{
    std::uint64_t event_id = 0;
    std::uint64_t device_id = 0;
    std::uint64_t timestamp_unix_ms = 0;
    std::uint64_t type = 0;
    std::uint64_t payload_version = 0;
};

ParsedDeviceEvent ParseAndValidateDeviceEventBlob(const std::string& blob)
{
    gundam::v1::DeviceEvent event;
    if (!event.ParseFromString(blob))
    {
        throw std::runtime_error("invalid protobuf payload in event_blob");
    }

    if (event.payload_case() == gundam::v1::DeviceEvent::PAYLOAD_NOT_SET)
    {
        throw std::runtime_error("event_blob has no payload oneof field");
    }

    if (event.type() == gundam::v1::UNSPECIFIED)
    {
        throw std::runtime_error("event_blob has unspecified type");
    }

    return ParsedDeviceEvent{
        static_cast<std::uint64_t>(event.event_id()),
        static_cast<std::uint64_t>(event.device_id()),
        static_cast<std::uint64_t>(event.timestamp_unix_ms()),
        static_cast<std::uint64_t>(event.type()),
        static_cast<std::uint64_t>(event.payload_version()),
    };
}

void PublishDeviceEvent(
    sw::redis::Redis& redis,
    const char*       stream,
    const DeviceEvent& row_event)
{
    const auto parsed = ParseAndValidateDeviceEventBlob(row_event.encoded_event_blob);

    using Field = std::pair<std::string, std::string>;
    std::vector<Field> fields;
    fields.reserve(8);
    fields.emplace_back("row_id", std::to_string(row_event.id));
    fields.emplace_back("event_id", std::to_string(parsed.event_id));
    fields.emplace_back("device_id", std::to_string(parsed.device_id));
    fields.emplace_back("timestamp_unix_ms", std::to_string(parsed.timestamp_unix_ms));
    fields.emplace_back("type", std::to_string(parsed.type));
    fields.emplace_back("payload_version", std::to_string(parsed.payload_version));
    fields.emplace_back("payload_format", "protobuf");
    fields.emplace_back("event_proto", row_event.encoded_event_blob);

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
