#include "proto/device_event.pb.h"
#include <sw/redis++/redis.h>
#include "EventPublisher.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace gundam::v1;

EventPublisher::EventPublisher(
    Config& config,
    SqliteDeviceStore& storage,
    sw::redis::Redis& redis)
    : config_(config),
      storage_(storage),
      redis_(redis)
{}

void
EventPublisher::Run()
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
            
            gundam::v1::DeviceEvent msg;
            // validade blob
            if (!msg.ParseFromString(row->encoded_event_blob))
                std::cerr << "[ERR] :: EventPublisher :: Could not parse event id: " << row->id << "\n";    
            
            std::cout << msg.DebugString() << std::endl;

            auto blob = msg.SerializeAsString();
            auto data = std::make_pair(std::string("data"), blob); 

            redis_.xadd(config_.redis_stream, "*", {data});
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

