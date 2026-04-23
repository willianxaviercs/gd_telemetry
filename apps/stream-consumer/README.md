# Consumer

Minimal C++ service placeholder for:

- reading protobuf events from Redis Streams
- transforming them into relational writes
- persisting them into Postgres

Current status: the entrypoint is still minimal, but it now builds through the repo-level CMake project and links shared code from `shared/`.

Build from the repo root:

```bash
cmake -S . -B build
cmake --build build --target consumer
```
