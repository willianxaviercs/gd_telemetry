#include "StreamConsumer.h"
#include "Storage.h"
#include "Config.h"

#include <sw/redis++/redis++.h>
#include <pqxx/pqxx>

#include <iostream>
#include <format>

using namespace sw::redis;

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
        ConnectionOptions redis_opts;
        redis_opts.host = redis_config.host;
        redis_opts.port = redis_config.port;
        sw::redis::Redis redis(redis_opts);

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

