#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap
runtime_load_topology
runtime_require_command python3

edge_daemon_binary="${repo_root}/build/edge-daemon/edge-daemon"

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"
: "${SIMULATOR_INTERVAL_SECONDS:?SIMULATOR_INTERVAL_SECONDS is required}"
: "${EDGE_DAEMON_POLL_INTERVAL_SECONDS:?EDGE_DAEMON_POLL_INTERVAL_SECONDS is required}"
: "${REDIS_HOST:?REDIS_HOST is required}"
: "${REDIS_PORT:?REDIS_PORT is required}"
: "${REDIS_STREAM:?REDIS_STREAM is required}"

if [[ ! -x "${edge_daemon_binary}" ]]; then
    echo "missing edge-daemon binary: ${edge_daemon_binary}" >&2
    echo "build it first with: make edge-daemon-build" >&2
    exit 1
fi

mkdir -p "${devices_dir}" "${run_dir}"

for pid_file in "${run_dir}"/*.pid; do
    [[ -e "${pid_file}" ]] || break

    pid="$(cat "${pid_file}")"
    if [[ -n "${pid}" ]] && kill -0 "${pid}" 2>/dev/null; then
        echo "runtime already running; stop it first with: make runtime-stop" >&2
        exit 1
    fi
done

for ((offset = 0; offset < DEVICE_COUNT; offset++)); do
    device_id=$((DEVICE_ID_START + offset))
    device_dir="${devices_dir}/${device_id}"
    db_path="${device_dir}/device.db"
    simulator_log_path="${device_dir}/simulator.log"
    edge_daemon_log_path="${device_dir}/edge-daemon.log"
    simulator_pid_path="${run_dir}/simulator-${device_id}.pid"
    edge_daemon_pid_path="${run_dir}/edge-daemon-${device_id}.pid"

    if [[ ! -f "${db_path}" ]]; then
        echo "missing sqlite database for device ${device_id}: ${db_path}" >&2
        echo "prepare runtime state first with: make runtime-prepare" >&2
        exit 1
    fi

    python3 "${repo_root}/simulator/main.py" \
        --device-id "${device_id}" \
        --interval-seconds "${SIMULATOR_INTERVAL_SECONDS}" \
        >"${simulator_log_path}" 2>&1 &
    echo "$!" > "${simulator_pid_path}"
    echo "started simulator for device ${device_id}, pid=$(cat "${simulator_pid_path}")"

    "${edge_daemon_binary}" \
        --device-db-path "${db_path}" \
        --poll-interval-seconds "${EDGE_DAEMON_POLL_INTERVAL_SECONDS}" \
        --redis-stream "${REDIS_STREAM}" \
        --redis-host "${REDIS_HOST}" \
        --redis-port "${REDIS_PORT}" \
        >"${edge_daemon_log_path}" 2>&1 &
    echo "$!" > "${edge_daemon_pid_path}"
    echo "started edge-daemon for device ${device_id}, pid=$(cat "${edge_daemon_pid_path}")"
done
