#pragma once

struct Config
{
    double      poll_interval_seconds;
    const char* db_path;
    const char* redis_host;
    const char* redis_stream;
    int         redis_port;
};

void Usage(const char* program_name);
bool ParseArgs(int argc, char** argv, Config& config);
