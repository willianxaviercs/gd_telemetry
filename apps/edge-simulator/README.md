# Simulator

Local per-device process for:

- generating fake device events
- writing them into the device SQLite database

## SQLite Schema

The local database schema lives in [schema/sqlite/device_events.sql](/home/xavier/programming/agent-coding/gundam/schema/sqlite/device_events.sql).

It stays close to the protobuf envelope while keeping SQLite practical for polling:

- `id` is the local SQLite row identifier that the edge daemon can poll on
- `device_id`, `timestamp_unix_ms`, and `type` map directly to `DeviceEvent`
- `temperature_celsius` maps to `TemperatureReading.celsius`
- `status` maps to `StatusUpdate.status`

The table stays flat on purpose so the edge daemon can convert one SQLite row into one protobuf event with minimal reshaping.

## Usage

The simulator derives the SQLite path from the device ID and the repo runtime layout:

```bash
python3 simulator/main.py --device-id 0 --interval-seconds 60
```

That writes to:

```text
runtime/devices/0/device.db
```

It inserts one random event every `--interval-seconds`.

```bash
python3 simulator/main.py --device-id 0 --interval-seconds 5
```

When started through the runtime scripts, the interval comes from `SIMULATOR_INTERVAL_SECONDS` in [runtime/topology/devices.env](/home/xavier/programming/agent-coding/gundam/runtime/topology/devices.env:1).
