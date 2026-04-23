# Edge Daemon

Local per-device C++ process for the edge runtime.

Current responsibility:

- reading local device data from SQLite
- converting rows into event-shaped records
- publishing events into Redis Streams

## Current Status

The current event-publishing responsibility polls the local SQLite database, appends each new row into a Redis Stream through `hiredis`, and keeps polling state inside the same SQLite DB using `collector_state.last_published_id`.

That means restarts resume from the last successfully published row within the same device database.

## Usage

Build from the repo root:

```bash
cmake -S . -B build
cmake --build build --target edge-daemon
```

Run with an explicit SQLite path:

```bash
./build/edge-daemon/edge-daemon \
  --device-db-path runtime/devices/0/device.db \
  --poll-interval-seconds 1 \
  --redis-stream device_events
```

Optional Redis connection arguments:

```bash
./build/edge-daemon/edge-daemon \
  --device-db-path runtime/devices/0/device.db \
  --poll-interval-seconds 1 \
  --redis-stream device_events \
  --redis-host 127.0.0.1 \
  --redis-port 6379
```

## Dependencies

- `sqlite3` development headers and libraries must be available either from the system or `vendor/sqlite3/`
- `hiredis` development headers and libraries must be available either from the system or `vendor/hiredis/`
- shared protobuf C++ code generation is configured centrally under `shared/` when both `protoc` and the protobuf C++ runtime are available
