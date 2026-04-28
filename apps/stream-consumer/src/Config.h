#pragma once

struct ConsumerConfig
{
    const char* stream;
    const char* group;
    const char* name;
};

struct RedisConfig
{
    const char* host;
    int   port;
};

struct PostgresConfig
{
    const char* host;
    const char* db;
    const char* user;
    const char* password;
    int port;
};

RedisConfig    LoadRedisConfig(void);
PostgresConfig LoadPostgresConfig(void);
ConsumerConfig LoadConsumerConfig(void);

