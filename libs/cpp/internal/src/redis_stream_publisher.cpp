#include "core/redis_stream_publisher.h"

#include "hiredis/hiredis.h"
#include <string>
#include <stdexcept>
#include <vector>

class RedisReplyHandle {
  public:
    explicit RedisReplyHandle(redisReply* reply) : reply_(reply) {}
    ~RedisReplyHandle() {
        if (reply_ != nullptr) {
            freeReplyObject(reply_);
        }
    }

    redisReply* get() const { return reply_; }

  private:
    redisReply* reply_;
};

std::string EventTypeValue(const DeviceEvent& event) {
    return std::to_string(static_cast<int>(event.type));
}

RedisStreamPublisher::RedisStreamPublisher(const std::string& host, int port) : context_(nullptr) {
    context_ = redisConnect(host.c_str(), port);
    if (context_ == nullptr) {
        throw std::runtime_error("connect redis: redisConnect returned null");
    }

    if (context_->err != 0) {
        const std::string error_message = context_->errstr != nullptr ? context_->errstr : "unknown";
        redisFree(context_);
        context_ = nullptr;
        throw std::runtime_error("connect redis: " + error_message);
    }
}

RedisStreamPublisher::~RedisStreamPublisher() {
    if (context_ != nullptr) {
        redisFree(context_);
    }
}

std::string RedisStreamPublisher::Publish(const std::string& stream_name, const DeviceEvent& event) {
    std::vector<std::string> args{
        "XADD",
        stream_name,
        "*",
        "event_id",
        std::to_string(event.id),
        "device_id",
        std::to_string(event.device_id),
        "timestamp_unix_ms",
        std::to_string(event.timestamp),
        "type",
        EventTypeValue(event),
    };

    if (event.type == EventType::TemperatureReading) {
        args.emplace_back("temperature_celsius");
        args.emplace_back(std::to_string(event.temperature_celsius));
    } else if (event.type == EventType::StatusUpdate) {
        args.emplace_back("status");
        args.emplace_back(std::to_string(static_cast<int>(event.device_status)));
    }

    std::vector<const char*> argv;
    std::vector<size_t> argvlen;
    argv.reserve(args.size());
    argvlen.reserve(args.size());

    for (const std::string& arg : args) {
        argv.push_back(arg.c_str());
        argvlen.push_back(arg.size());
    }

    RedisReplyHandle reply(static_cast<redisReply*>(
        redisCommandArgv(context_, static_cast<int>(argv.size()), argv.data(), argvlen.data())));

    if (reply.get() == nullptr) {
        const std::string error_message = context_->errstr != nullptr ? context_->errstr : "unknown";
        throw std::runtime_error("publish redis stream: " + error_message);
    }

    if (reply.get()->type == REDIS_REPLY_ERROR) {
        throw std::runtime_error("publish redis stream: " + std::string(reply.get()->str));
    }

    if ((reply.get()->type == REDIS_REPLY_STRING || reply.get()->type == REDIS_REPLY_STATUS) &&
        reply.get()->str != nullptr) {
        return reply.get()->str;
    }

    throw std::runtime_error("publish redis stream: unexpected reply type");
}

