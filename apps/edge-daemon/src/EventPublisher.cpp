#include "EventPublisher.h"

EventPublisher::EventPublisher(
    Config& config,
    SqliteDeviceStore& storage,
    RedisStreamPublisher& redis)
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

            redis_.Publish(config_.redis_stream, *next_event);
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
