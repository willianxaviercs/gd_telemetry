# Telemetry

Minimal distributed-system scaffold

## Structure

```
.
├── apps             # every application
├── CMakeLists.txt   # project Cmake
├── docs             # documentation
├── libs             # shared libraries
├── Makefile         # development workflow
├── platform         # infrastructure
├── README.md        # this document
└── vcpkg.json       # project dependencies
```

## Development

```bash
make build
```

Builds everything that needs to be build.

```bash
make run
```
Runs local development environment.

```bash
make stop
```

Stop and clean local development environment.
