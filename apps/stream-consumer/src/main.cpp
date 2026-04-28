#include <iostream>
#include <format>

#include <sw/redis++/redis++.h>
#include <pqxx/pqxx>

#include "StreamConsumer.h"
#include "Storage.h"
#include "proto/device_event.pb.h"
#include "Config.h"

int main(void)
{
    try
    {
        // postgres
        auto pq_config = LoadPostgresConfig();
        auto conn_str =
            std::format("host={} port={} dbname={} user={} password={}",
            pq_config.host, pq_config.port, pq_config.db, pq_config.user, pq_config.password);

        Storage storage(conn_str);

        // redis
        auto redis_config = LoadRedisConfig();
        auto redis_uri = std::format("tcp://{}:{}", redis_config.host, redis_config.port);

        sw::redis::Redis redis(redis_uri);

        // consumer
        auto consumer_config = LoadConsumerConfig();

        StreamConsumer consumer(consumer_config, redis, storage);
        consumer.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    return 0;
}

