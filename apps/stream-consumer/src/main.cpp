#include <iostream>
#include <cstdlib>
#include <format>

#include <sw/redis++/redis++.h>
#include <pqxx/pqxx>

#include "proto/device_event.pb.h"
#include "Config.cpp"

using namespace sw::redis;

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

        pqxx::work tx(conn_);
        tx.exec(
            "INSERT INTO hello_world (message) VALUES ($1)",
            pqxx::params{"hello from consumer"}
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

// ---------- Debug print ----------
void DebugPrintEvent(const gundam::v1::DeviceEvent& e)
{
    std::cout
        << "event_id:  " << e.event_id() << " "
        << "device_id: " << e.device_id() << " "
        << "timestamp: " << e.timestamp_unix_ms() << " "
        << "type:      " << e.type() << " "
        << "payload_v: " << e.payload_version()
        << std::endl;

    switch (e.payload_case())
    {
        case gundam::v1::DeviceEvent::kMissionUpdate:
        {
            const auto& m = e.mission_update();
            std::cout
                << "mission_state: " << m.state() << "\n"
                << "reason:        " << m.reason() << "\n"
                << std::endl;
            break;
        }

        case gundam::v1::DeviceEvent::kPositionSample:
        {
            const auto& m = e.position_sample();
            std::cout
                << "latitude_deg:  " << m.latitude_deg() << "\n"
                << "longitude_deg: " << m.longitude_deg() << "\n"
                << "altitude_m:    " << m.altitude_m() << "\n"
                << "heading_deg:   " << m.heading_deg() << "\n"
                << "speed_mps:     " << m.speed_mps() << "\n"
                << std::endl;
            break;
        }

        case gundam::v1::DeviceEvent::kHealthSample:
        {
            const auto& m = e.health_sample();
            std::cout
                << "battery_pct:      " << m.battery_pct() << "\n"
                << "link_quality_pct: " << m.link_quality_pct() << "\n"
                << "gps_fix_type:     " << m.gps_fix_type() << "\n"
                << std::endl;
            break;
        }

        default:
            break;
    }
}

int main()
{
    auto config_pq    = LoadPostgresConfig();
    auto conn_str = std::format(
        "host={} port={} dbname={} user={} password={}",
        config_pq.host, config_pq.port, config_pq.db, config_pq.user, config_pq.password
    );

    auto config_redis = LoadRedisConfig();
    auto redis_uri = std::format("tcp://{}:{}", config_redis.host, config_redis.port);

    Redis redis(redis_uri);    
    Storage storage(conn_str);

    try {
        redis.xgroup_create(config_redis.stream, config_redis.group, "$", true);
    } catch (...) {}

    // ---------- CORRECT TYPE ----------
    using Field = std::pair<std::string, std::string>;
    using Fields = std::vector<Field>;

    using StreamEntry = std::pair<std::string, Fields>;
    using ItemStream  = std::vector<StreamEntry>;

    using Result = std::unordered_map<std::string, ItemStream>;

    Result result;

    // ---------- LOOP ----------
    while (true)
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

                    DebugPrintEvent(msg);
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