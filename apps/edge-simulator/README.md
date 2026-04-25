# Simulator

Local per-device process for:

- generating fake device events
- writing them into the device SQLite database

The simulator uses generated Python protobuf classes from `gen/device_event_pb2.py`.

## SQLite Schema

The local database schema lives in [libs/schemas/sqlite/device_events.sql](/home/xavier/programming/agent-coding/gundam/libs/schemas/sqlite/device_events.sql).

It stores full serialized `DeviceEvent` protobuf messages in SQLite:

- `id` is the local SQLite row identifier that the edge daemon can poll on
- `event_blob` is a protobuf-encoded `DeviceEvent`

The simulator currently emits three event types for a basic autonomous drone:
- position samples
- health samples
- mission updates

## Usage

The simulator derives the SQLite path from the device ID and the repo runtime layout:

```bash
python3 simulator/main.py --device-id 0 --interval-seconds 60
```

That writes to:

```text
runtime/devices/0/device.db
```

It inserts synthetic drone events every `--interval-seconds`.

```bash
python3 simulator/main.py --device-id 0 --interval-seconds 5
```

When started through the runtime scripts, the interval comes from `SIMULATOR_INTERVAL_SECONDS` in [runtime/topology/devices.env](/home/xavier/programming/agent-coding/gundam/runtime/topology/devices.env:1).

## Python dependency

The simulator requires the Python protobuf runtime:

```bash
python3 -m pip install --break-system-packages protobuf
```
