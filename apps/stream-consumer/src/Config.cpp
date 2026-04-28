#include "Config.h"

#include <cstdlib>
#include <stdexcept>

ConsumerConfig LoadConsumerConfig(void)
{
    ConsumerConfig cfg{};

    cfg.stream = std::getenv("REDIS_STREAM");
    cfg.group  = std::getenv("REDIS_GROUP");
    cfg.name   = std::getenv("REDIS_CONSUMER");

    if (!cfg.stream || !cfg.group || !cfg.name)
        throw std::runtime_error("Missing consumer env vars");

    return cfg;
}

RedisConfig LoadRedisConfig(void)
{
    RedisConfig cfg{};

    cfg.host         = std::getenv("REDIS_HOST");
    const char* port = std::getenv("REDIS_PORT");

    if (!cfg.host || !port)
        throw std::runtime_error("Missing Redis env vars");

    cfg.port = std::atoi(port);

    if (cfg.port <= 0 || cfg.port > 65535)
        throw std::runtime_error("Invalid REDIS_PORT");

    return cfg;
}

PostgresConfig LoadPostgresConfig(void)
{
    PostgresConfig cfg{};

    cfg.host     = std::getenv("DB_HOST");
    cfg.db       = std::getenv("DB_NAME");
    cfg.user     = std::getenv("DB_USER");
    cfg.password = std::getenv("DB_PASSWORD");

    const char* port = std::getenv("DB_PORT");

    if (!cfg.host || !cfg.db || !cfg.user || !cfg.password || !port)
        throw std::runtime_error("Missing Postgres env vars");

    cfg.port = std::atoi(port);

    if (cfg.port <= 0 || cfg.port > 65535)
        throw std::runtime_error("Invalid PG_PORT");

    return cfg;
}

