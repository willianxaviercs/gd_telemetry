#include "Storage.h"

Storage::Storage(const std::string& conn_str)
        : conn_str_(conn_str)
        , conn_(conn_str)
{
}

void
Storage::InsertEvent(const gundam::v1::DeviceEvent& e)
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

void
Storage::EnsureConnection(void)
{
    if (conn_.is_open())
        return;

    std::cerr << "reconnecting" << std::endl;
    conn_ = pqxx::connection(conn_str_);
}

