#pragma once

#include <cstdint>
#include <optional>
#include <string>

enum class EventType : int
{
    Unspecified,
    TemperatureReading,
    StatusUpdate,
};

enum class DeviceStatus : int
{
    Unspecified,
    Online,
    Offline,
    Error,
};

struct DeviceEvent
{
    int64_t id;
    int64_t device_id;
    int64_t timestamp;
    EventType type;
    union
    {
        double temperature_celsius;
        DeviceStatus device_status;
    };
};

