#pragma once

#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include "proto/device_event.pb.h"

class Storage
{
    const std::string conn_str_;
    pqxx::connection conn_;
public:
    Storage(const std::string& conn_str);

    void InsertEvent(const gundam::v1::DeviceEvent& e);

private:
    void EnsureConnection(void);
};

