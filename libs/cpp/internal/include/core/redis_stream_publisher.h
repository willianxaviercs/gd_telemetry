#pragma once

#include <string>

#include "device_event.h"

struct redisContext;

class RedisStreamPublisher {
  public:
    RedisStreamPublisher(const std::string& host, int port);
    ~RedisStreamPublisher();

    RedisStreamPublisher(const RedisStreamPublisher&) = delete;
    RedisStreamPublisher& operator=(const RedisStreamPublisher&) = delete;

    std::string Publish(const std::string& stream_name, const DeviceEvent& event);

  private:
    redisContext* context_;
};

