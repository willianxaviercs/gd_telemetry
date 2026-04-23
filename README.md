# Gundam Monorepo Skeleton

Minimal distributed-system scaffold based on the design doc.

## Structure

```text
.
├── CMakeLists.txt
├── api
│   ├── README.md
│   ├── package.json
│   └── src
│       └── server.js
├── edge-daemon
│   ├── CMakeLists.txt
│   ├── README.md
│   └── src
│       └── main.cpp
├── consumer
│   ├── CMakeLists.txt
│   ├── README.md
│   └── src
│       └── main.cpp
├── design-doc.md
├── infra
│   ├── .env.example
│   └── docker-compose.yml
├── proto
│   ├── README.md
│   └── device_event.proto
├── runtime
│   ├── README.md
│   ├── bin
│   ├── devices
│   └── topology
├── shared
│   ├── CMakeLists.txt
│   ├── include
│   └── src
├── schema
│   └── sqlite
│       └── device_events.sql
├── vendor
│   ├── README.md
│   ├── hiredis
│   └── sqlite3
└── simulator
    ├── README.md
    └── main.py
```

## Notes

- This repo only provides the initial skeleton.
- C++ applications build from the repo root through CMake subdirectories.
- Shared C++ code now lives under `shared/`, including helpers and protobuf codegen wiring.
- Native dependencies are intended to be shared from `vendor/`, with system installs used as a fallback when available.

## Local Dev

Primary local workflow:

```bash
make up
```

That will:

- build the edge daemon
- start Redis and Postgres with Docker Compose
- prepare per-device SQLite state under `runtime/devices/`
- start simulators and edge daemons in the background

To stop everything and remove local runtime plus Redis/Postgres data:

```bash
make down
```

To stop everything but keep Redis/Postgres volumes:

```bash
make down-keep
```

To build both C++ applications directly:

```bash
make cpp-build
```
