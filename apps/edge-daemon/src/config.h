#include <string>
#include <iostream>
#include <vector>

struct Config
{
    double poll_interval_seconds;
    const char* db_path;
    const char* redis_host;
    const char* redis_stream;
    int redis_port;
};

struct Opt
{
    const char* flag;
    const char* desc;
    bool  is_set;
    void (*set)(Config&, const char*);
};

static Opt opts[] = {
    { 
        .flag = "--device-db-path", .desc =  "<path>",    
        .set = [](Config& c, const char* v)
        {
            c.db_path = v;
        },
    },
    { 
        .flag = "--poll-interval-seconds", .desc =  "<seconds>",
        .set = [](Config& c, const char* v)
        {
            c.poll_interval_seconds = std::stod(v);
        },
    },
    {
        .flag = "--redis-stream", .desc =  "<name>",
        .set = [](Config& c, const char* v)
        {
            c.redis_stream = v;
        }
    },
    {
        .flag = "--redis-host", .desc =  "<host>",
        .set = [](Config& c, const char* v)
        {
            c.redis_host = v;
        }
    },
    {
        .flag = "--redis-port", .desc =  "<port>",
        .set = [](Config& c, const char* v)
        {
            c.redis_port = std::stoi(v);
        }
    },
};

void usage(const char* program_name)
{
    std::cerr << "usage: " << program_name << " ";
    for (const auto& o : opts)
        std::cerr << o.flag << " " << o.desc << " ";

    std::cerr << "\n";
}

bool parse_args(int argc, char** argv, Config& config)
{
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg = argv[i];

        for (auto& o : opts)
        {
            if (arg == o.flag)
            {
                if (i + 1 >= argc)
                    return false;

                o.set(config, argv[++i]);
                o.is_set = true;
                break;
            }
        }
    }

    for (const auto& o : opts)
    {
        if (!o.is_set)
        {
            std::cerr << "argument missing: " << o.flag << "\n";
            return false;
        }
    }

    return true;
}

