# TODO

- Vendor native dependencies instead of relying on system-installed development packages:
  - `sqlite3`
  - `hiredis`
  - Postgres client library
- Split the current edge-daemon responsibility into internal classes starting with `EventPublisher`
- Add `runtime/bin/run-edge-daemons.sh` or rename the current runtime start flow accordingly
- Replace the temporary direct Redis RESP client with vendored static `hiredis`
- Define the Redis Stream contract:
  - stream name
  - entry field layout
  - edge-daemon and consumer checkpoint strategy
- Implement the consumer read path from Redis Streams
- Define the Postgres schema derived from protobuf
- Implement the API read path from Postgres
