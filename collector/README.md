# Collector

Local per-device C++ process for:

- reading local device data from SQLite
- converting rows into event-shaped records
- eventually publishing events into Redis Streams

## Current Status

The collector currently polls the local SQLite database and prints each new row to stdout.

It now appends each new row into a Redis Stream and keeps polling state inside the same SQLite DB using `collector_state.last_published_id`, so restarts resume from the last successfully published row.

## Usage

The collector reads from an explicit SQLite path:

```bash
./build/collector/collector \
  --device-db-path runtime/devices/0/device.db \
  --poll-interval-seconds 1 \
  --redis-stream device_events
```

Optional Redis connection arguments:

```bash
./build/collector/collector \
  --device-db-path runtime/devices/0/device.db \
  --poll-interval-seconds 1 \
  --redis-stream device_events \
  --redis-host 127.0.0.1 \
  --redis-port 6379
```
