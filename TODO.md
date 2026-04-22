# TODO

- Vendor native dependencies instead of relying on system-installed development packages:
  - `sqlite3`
  - `hiredis`
  - Postgres client library
- Add `runtime/bin/run-collectors.sh`
- Extend `runtime/bin/up.sh` to start collectors alongside simulators
- Replace collector stdout printing with Redis Stream publish
- Define the Redis Stream contract:
  - stream name
  - entry field layout
  - collector and consumer checkpoint strategy
- Implement the consumer read path from Redis Streams
- Define the Postgres schema derived from protobuf
- Implement the API read path from Postgres
