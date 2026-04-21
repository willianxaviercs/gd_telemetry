CREATE TABLE IF NOT EXISTS device_events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id INTEGER NOT NULL,
    timestamp_unix_ms INTEGER NOT NULL,
    type INTEGER NOT NULL,
    temperature_celsius REAL,
    status INTEGER,
    CHECK (type IN (1, 2)),
    CHECK (
        (type = 1 AND temperature_celsius IS NOT NULL AND status IS NULL) OR
        (type = 2 AND status IS NOT NULL AND temperature_celsius IS NULL)
    )
);

CREATE INDEX IF NOT EXISTS idx_device_events_device_id
    ON device_events (device_id);

CREATE INDEX IF NOT EXISTS idx_device_events_timestamp_unix_ms
    ON device_events (timestamp_unix_ms);

CREATE INDEX IF NOT EXISTS idx_device_events_type
    ON device_events (type);
