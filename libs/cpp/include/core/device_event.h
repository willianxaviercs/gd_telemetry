#pragma once

#include <cstdint>
#include <string>

using std::string;

enum class EventType : int32_t
{
    UNSPECIFIED,
    POSITION_SAMPLE,
    HEALTH_SAMPLE,
    MISSION_UPDATE,
};

struct DeviceEvent
{
    int64_t   id;
    int64_t   device_id;
    int64_t   timestamp;
    EventType type;
    string    payload;
};

