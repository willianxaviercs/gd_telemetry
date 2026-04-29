#include "Storage.h"

Storage::Storage(const std::string& conn_str)
        : conn_str_(conn_str)
        , conn_(conn_str)
{}

void Storage::InsertEvent(
        const Payload& p,
        const std::string& device_id,
        const std::string& timestamp,
        const std::string& type)
{
    EnsureConnection();

    std::string payload = p.SerializeAsString();

    pqxx::work tx(conn_);
    tx.exec(
        "INSERT INTO device_events "
        "(device_id, timestamp, type, payload) "
        "VALUES ($1, $2, $3, $4)",
        pqxx::params(
            std::stoll(device_id),
            std::stoll(timestamp),
            std::stoll(type),
            pqxx::binary_cast(payload)
        )
    );
    tx.commit();
}

void Storage::EnsureConnection(void)
{
    if (conn_.is_open())
        return;

    std::cerr << "reconnecting" << std::endl;
    conn_ = pqxx::connection(conn_str_);
}

