# Runtime

Linux-oriented local runtime area for development and simulation.

## Purpose

This directory is for generated local state and lightweight orchestration artifacts:

- per-device SQLite databases
- local logs or temporary runtime files
- checked-in topology definitions
- helper scripts for creating and running many device instances

## Layout

```text
runtime/
  bin/
  devices/
  topology/
```

`runtime/devices/` is intended to be mounted as `tmpfs` on Linux during development so device artifacts stay inside the repo without polluting the machine.

Generated contents under `runtime/devices/` are ignored by git.

## Topology

The checked-in topology file is [runtime/topology/devices.env](/home/xavier/programming/agent-coding/gundam/runtime/topology/devices.env).

It currently defines:

- `DEVICE_ID_START`
- `DEVICE_COUNT`
- `SIMULATOR_INTERVAL_SECONDS`
- `COLLECTOR_POLL_INTERVAL_SECONDS`
- `REDIS_HOST`
- `REDIS_PORT`
- `REDIS_STREAM`

Device directories are derived from those values as `runtime/devices/<id>/`.

## Scripts

- [runtime/bin/base.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/base.sh) provides shared runtime bootstrap and common topology/command checks
- [runtime/bin/prepare.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/prepare.sh) recreates per-device SQLite state from the topology and schema
- [runtime/bin/start.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/start.sh) starts simulators and collectors in the background and stores PID files under `runtime/run/`
- [runtime/bin/stop.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/stop.sh) stops background simulators and collectors tracked in `runtime/run/`
- [runtime/bin/mount-tmpfs.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/mount-tmpfs.sh) mounts `runtime/devices/` as `tmpfs`
- [runtime/bin/unmount-tmpfs.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/unmount-tmpfs.sh) unmounts `runtime/devices/`
- [runtime/bin/up.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/up.sh) is a thin compatibility wrapper around `prepare.sh` and `start.sh`
- [runtime/bin/clean.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/clean.sh) removes generated device directories and PID files when `runtime/devices/` is not mounted

Typical flow:

```bash
make up
```

With Redis already running on the configured host/port, `prepare.sh` and `start.sh` will:

- create `runtime/devices/<id>/`
- initialize one SQLite database per device
- start one simulator and one collector per device
- keep writing simulator logs and collector logs inside each device directory
- write PID files under `runtime/run/`

Cleanup flow:

```bash
make down
```

Manual simulator flow:

```bash
make collector-build
docker compose -f infra/docker-compose.yml up -d --wait redis postgres
./runtime/bin/prepare.sh
./runtime/bin/start.sh
```

To stop runtime processes without touching Redis or Postgres:

```bash
./runtime/bin/stop.sh
```
