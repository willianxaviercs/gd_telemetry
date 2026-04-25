# Simulator

Edge device-simulator for development purposes

- Generating fake device events
- Writing them into local device storage

## Behavior

The simulator currently emits three event types for a basic autonomous drone:
- position samples
- health samples
- mission updates

## Usage

The simulator derives the SQLite path from the device ID and the repo runtime layout:

```bash
python3 simulator/main.py \
    --device-id <device_id> \
    --interval-seconds <seconds> \
    --db_path </path/to/device.db>
```

## Dependencies

The simulator requires the Python protobuf runtime:

```bash
python3 -m pip install --break-system-packages protobuf
```
