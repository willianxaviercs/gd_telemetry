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
