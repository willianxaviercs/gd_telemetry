CREATE TABLE IF NOT EXISTS device_events (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id INTEGER NOT NULL,
    timestamp INTEGER NOT NULL,
    type      INTEGER NOT NULL,
    payload   BLOB    NOT NULL,
    CHECK (length(payload) > 0)
);

CREATE TABLE IF NOT EXISTS collector_state (
    id INTEGER PRIMARY KEY CHECK (id = 1),
    last_published_id INTEGER NOT NULL
);

INSERT INTO collector_state (id, last_published_id)
VALUES (1, 0)
ON CONFLICT(id) DO NOTHING;
