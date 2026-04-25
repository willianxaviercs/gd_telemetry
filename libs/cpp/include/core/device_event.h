#pragma once

#include <cstdint>
#include <string>

struct DeviceEvent
{
    int64_t     id;
    std::string encoded_event_blob;
};

