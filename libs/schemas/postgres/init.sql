CREATE TABLE IF NOT EXISTS device_events (
    id         BIGSERIAL PRIMARY KEY,
    device_id  BIGINT    NOT NULL,
    timestamp  BIGINT    NOT NULL,
    type       BIGINT    NOT NULL,
    payload    BYTEA NOT NULL,
    CHECK (length(payload) > 0)
);

