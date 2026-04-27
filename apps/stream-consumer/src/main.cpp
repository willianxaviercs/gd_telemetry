#include <iostream>
#include <cstdlib>
#include <format>

#include <sw/redis++/redis++.h>
#include <pqxx/pqxx>

#include "proto/device_event.pb.h"
#include "Config.cpp"

using namespace sw::redis;

using Field = std::pair<std::string, std::string>;
using Fields = std::vector<Field>;
using StreamEntry = std::pair<std::string, Fields>;
using ItemStream  = std::vector<StreamEntry>;
using RedisStreamResult = std::unordered_map<std::string, ItemStream>;

// TODO: pull it to separate file
class Storage
{
    const std::string conn_str_;
    pqxx::connection conn_;
public:
    Storage(const std::string& conn_str)
        : conn_str_(conn_str)
        , conn_(conn_str)
    {}

    void InsertEvent(const gundam::v1::DeviceEvent& e)
    {
        EnsureConnection();

        std::string data = e.SerializeAsString();

        pqxx::work tx(conn_);
        tx.exec(
            "INSERT INTO device_events (event_blob) VALUES ($1)",
            pqxx::binary_cast(data)
        );
        tx.commit();
    }

private:
    void EnsureConnection(void)
    {
        if (conn_.is_open())
            return;

        std::cerr << "reconnecting" << std::endl;
        conn_ = pqxx::connection(conn_str_);
    }
};

int main(void)
{
    try
    {
        // postgres setup
        auto config_pq    = LoadPostgresConfig();
        auto conn_str = std::format(
            "host={} port={} dbname={} user={} password={}",
            config_pq.host, config_pq.port, config_pq.db, config_pq.user, config_pq.password
        );
        Storage storage(conn_str);

        // redis setup
        auto config_redis = LoadRedisConfig();
        auto redis_uri = std::format("tcp://{}:{}", config_redis.host, config_redis.port);
        Redis redis(redis_uri);    
        redis.xgroup_create(config_redis.stream, config_redis.group, "$", true);
    }
    catch (const std::exception& e)
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    // TODO: pull this to an class
    // StreamConsumer consumer(redis, storare);
    // consumer.run();
    RedisStreamResult result;
    for (;;)
    {
        try
        {
            result.clear();

            redis.xreadgroup(
                config_redis.group,
                config_redis.consumer,
                config_redis.stream,
                ">",
                1,
                std::inserter(result, result.end())
            );

            for (const auto& [streamName, entries] : result)
            {
                for (const auto& entry : entries)
                {
                    std::cout << "processing id: " << entry.first << std::endl;

                    const auto& fields = entry.second;

                    if (fields.empty())
                        continue;

                    const auto& payload = fields.front().second;

                    gundam::v1::DeviceEvent msg;
                    msg.ParseFromString(payload);

                    std::cout << msg.DebugString() << std::endl;
                    storage.InsertEvent(msg);

                    redis.xack(config_redis.stream, config_redis.group, entry.first);
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "error: " << e.what() << std::endl;
        }
    }

    return 0;
}
