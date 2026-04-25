# Edge Daemon

Local per-device C++ process for the edge runtime.

Responsibility:

- Reading device events from local storage.
- Validating protobuf-encoded event blobs.
- Publishing events upstream into the cloud infrastructure.

## Usage

```bash
./edge-daemon \
  --device-db-path /path/to/device.db \
  --poll-interval-seconds <seconds>> \
  --redis-stream <stream_name>
  --redis-host <hostname> \
  --redis-port <port>
```
