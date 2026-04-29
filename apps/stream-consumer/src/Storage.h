#pragma once

#include <pqxx/pqxx>
#include "proto/payload.pb.h"

#include <iostream>
#include <string>

using namespace proto::v1;

class Storage
{
    const std::string conn_str_;
    pqxx::connection conn_;
public:
    Storage(const std::string& conn_str);

    void InsertEvent(
            const Payload& p,
            const std::string& device_id,
            const std::string& timestamp,
            const std::string& type);

private:
    void EnsureConnection(void);
};

