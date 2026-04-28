#pragma once

#include "Storage.h"
#include "Config.h"

#include <sw/redis++/redis++.h>

using Field = std::pair<std::string, std::string>;
using Fields = std::vector<Field>;
using StreamEntry = std::pair<std::string, Fields>;
using ItemStream  = std::vector<StreamEntry>;
using RedisStreamResult = std::unordered_map<std::string, ItemStream>;

class StreamConsumer
{
    ConsumerConfig& config_;
    sw::redis::Redis& redis_;
    Storage& storage_;
public:
    StreamConsumer(ConsumerConfig& config, sw::redis::Redis& redis, Storage& storage);

    void Run(void) const;
};

