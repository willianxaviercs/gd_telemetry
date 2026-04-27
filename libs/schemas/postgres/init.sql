CREATE TABLE IF NOT EXISTS device_events (
    id BIGSERIAL PRIMARY KEY,
    event_blob BYTEA NOT NULL,
    CHECK (length(event_blob) > 0)
);

