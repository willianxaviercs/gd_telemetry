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
- [runtime/bin/mount-tmpfs.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/mount-tmpfs.sh) mounts `runtime/devices/` as `tmpfs`
- [runtime/bin/unmount-tmpfs.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/unmount-tmpfs.sh) unmounts `runtime/devices/`
- [runtime/bin/create-devices.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/create-devices.sh) creates the per-device directories
- [runtime/bin/init-sqlite.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/init-sqlite.sh) initializes one SQLite database per device from the shared schema
- [runtime/bin/run-simulators.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/run-simulators.sh) starts one simulator per device and writes logs into each device directory
- [runtime/bin/run-collectors.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/run-collectors.sh) starts one collector per device and publishes new SQLite rows into Redis Streams
- [runtime/bin/up.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/up.sh) prepares runtime state, starts all simulators, and cleans up on exit
- [runtime/bin/clean.sh](/home/xavier/programming/agent-coding/gundam/runtime/bin/clean.sh) removes generated device directories when `runtime/devices/` is not mounted

Typical flow:

```bash
./runtime/bin/up.sh
```

With Redis already running on the configured host/port, `up.sh` will:

- create `runtime/devices/<id>/`
- initialize one SQLite database per device
- start one simulator and one collector per device
- keep writing simulator logs and collector logs inside each device directory
- clean generated device state on exit, while leaving Redis untouched

Cleanup flow:

```bash
./runtime/bin/unmount-tmpfs.sh
./runtime/bin/clean.sh
```

Manual simulator flow:

```bash
./runtime/bin/mount-tmpfs.sh
./runtime/bin/create-devices.sh
./runtime/bin/init-sqlite.sh
./runtime/bin/run-simulators.sh
./runtime/bin/run-collectors.sh
```

If mounting `tmpfs` is not desired for a given run, `up.sh` also supports:

```bash
RUNTIME_SKIP_TMPFS_MOUNT=1 ./runtime/bin/up.sh
```
