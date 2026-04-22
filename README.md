# Gundam Monorepo Skeleton

Minimal distributed-system scaffold based on the design doc.

## Structure

```text
.
├── api
│   ├── README.md
│   ├── package.json
│   └── src
│       └── server.js
├── collector
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
├── schema
│   └── sqlite
│       └── device_events.sql
└── simulator
    ├── README.md
    └── main.py
```

## Notes

- This repo only provides the initial skeleton.
- Service entrypoints are intentionally minimal.
- C++ services are set up to allow vendored native dependencies later.

## Local Dev

Primary local workflow:

```bash
make up
```

That will:

- build the collector
- start Redis and Postgres with Docker Compose
- prepare per-device SQLite state under `runtime/devices/`
- start simulators and collectors in the background

To stop everything and remove local runtime plus Redis/Postgres data:

```bash
make down
```

To stop everything but keep Redis/Postgres volumes:

```bash
make down-keep
```
