#include "proto/payload.pb.h"
#include "EventPublisher.h"

#include <sw/redis++/redis.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace proto::v1;

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
            const auto row = storage_.ReadNextEvent(last_published_id);

            if (!row.has_value())
            {
                std::this_thread::sleep_for(
                    std::chrono::duration<double>(config_.poll_interval_seconds));
                continue;
            }
            
            Payload p;
            if (!p.ParseFromString(row->payload))
                std::cerr << "[ERR] :: EventPublisher :: Could not parse event id: " << row->id << "\n";    
            
            std::cout << p.DebugString() << std::endl;

            auto blob = p.SerializeAsString();

            std::vector<std::pair<std::string, std::string>> data = {
                { "device_id", std::to_string(row->device_id) },
                { "timestamp", std::to_string(row->timestamp) },
                { "type"     , std::to_string(static_cast<int32_t>(row->type)) },
                { "payload"  , blob },
            };

            redis_.xadd(config_.redis_stream, "*", data.begin(), data.end());
            storage_.UpdateLastPublishedId(row->id);
        }
        catch (const std::exception& err)
        {
            std::cerr << "[ERR] :: EventPublisher :: " << err.what() << std::endl;
            std::cout << "[INF] :: EventPublisher :: Retrying in "
                      << RETRY_SECS << " seconds" << std::endl;

            std::this_thread::sleep_for(
                std::chrono::duration<double>(RETRY_SECS));
        }
    }
}

