#include <cstdlib>
#include <stdexcept>

struct RedisConfig
{
    const char* host;
    const char* stream;
    const char* group;
    const char* consumer;
    int port;
};

RedisConfig LoadRedisConfig(void)
{
    RedisConfig cfg{};

    cfg.host     = std::getenv("REDIS_HOST");
    cfg.stream   = std::getenv("REDIS_STREAM");
    cfg.group    = std::getenv("REDIS_GROUP");
    cfg.consumer = std::getenv("REDIS_CONSUMER");

    const char* port = std::getenv("REDIS_PORT");

    if (!cfg.host || !cfg.stream || !cfg.group || !cfg.consumer || !port)
        throw std::runtime_error("Missing Redis env vars");

    cfg.port = std::atoi(port);

    if (cfg.port <= 0)
        throw std::runtime_error("Invalid REDIS_PORT");

    return cfg;
}

struct PostgresConfig
{
    const char* host;
    const char* db;
    const char* user;
    const char* password;
    int port;
};

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
