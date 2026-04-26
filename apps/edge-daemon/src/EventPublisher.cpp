#include "EventPublisher.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "proto/device_event.pb.h"
#include <sw/redis++/redis.h>

EventPublisher::EventPublisher(
    Config& config,
    SqliteDeviceStore& storage,
    sw::redis::Redis& redis)
    : config_(config),
      storage_(storage),
      redis_(redis)
{}

/*
message DeviceEvent {
  uint64 event_id = 1;
  uint64 device_id = 2;
  uint64 timestamp_unix_ms = 3;
  EventType type = 4;
  uint32 payload_version = 5;

  oneof payload {
    PositionSample position_sample = 10;
    HealthSample health_sample = 11;
    MissionUpdate mission_update = 12;
  }
}
*/
using namespace gundam::v1;

void DebugPrintEvent(const gundam::v1::DeviceEvent& e)
{
    std::cout
        << "event_id:  " << e.event_id()          << " " 
        << "device_id: " << e.device_id()         << " "
        << "timestamp: " << e.timestamp_unix_ms() << " "
        << "type:      " << e.type()              << " "
        << "payload_v: " << e.payload_version() << std::endl;

    switch (e.payload_case())
    {
        case gundam::v1::DeviceEvent::kMissionUpdate:
        {
            const auto& m = e.mission_update();
            std::cout
                << "mission_state:" << m.state() << "\n"
                << "reason:      "  << m.reason() << "\n"
                << std::endl;
            break;
        }

        case gundam::v1::DeviceEvent::kPositionSample:
        {
            const auto& m = e.position_sample();
            std::cout
                << "latitude_deg:  " << m.latitude_deg()  << "\n"
                << "longitude_deg: " << m.longitude_deg() << "\n"
                << "altitude_m:    " << m.altitude_m()    << "\n"
                << "heading_deg:   " << m.heading_deg()   << "\n"
                << "speed_mps:     " << m.speed_mps()     << "\n"
                << std::endl;    
            break;
        }

        case gundam::v1::DeviceEvent::kHealthSample:
        {
            const auto& m = e.health_sample();
            std::cout
                << "battery_pct:      " << m.battery_pct()      << "\n"
                << "link_quality_pct: " << m.link_quality_pct() << "\n"
                << "gps_fix_type:     " << m.gps_fix_type()    << "\n"
                << std::endl;   
        }

        default:
            break;
    }
}

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
            
            gundam::v1::DeviceEvent msg;
            // validade blob
            if (!msg.ParseFromString(row->encoded_event_blob))
                std::cerr << "[ERR] :: EventPublisher :: Could not parse event id: " << row->id << "\n";    
            
            DebugPrintEvent(msg);
            auto blob = msg.SerializeAsString();
            auto data = std::make_pair(std::string("data"), blob); 

            // TODO(wx): domain stream name based on msg.type
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
