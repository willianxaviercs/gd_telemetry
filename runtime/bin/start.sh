#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap
runtime_load_topology
runtime_require_command python3

collector_binary="${repo_root}/collector/build/collector"

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"
: "${SIMULATOR_INTERVAL_SECONDS:?SIMULATOR_INTERVAL_SECONDS is required}"
: "${COLLECTOR_POLL_INTERVAL_SECONDS:?COLLECTOR_POLL_INTERVAL_SECONDS is required}"
: "${REDIS_HOST:?REDIS_HOST is required}"
: "${REDIS_PORT:?REDIS_PORT is required}"
: "${REDIS_STREAM:?REDIS_STREAM is required}"

if [[ ! -x "${collector_binary}" ]]; then
    echo "missing collector binary: ${collector_binary}" >&2
    echo "build it first with: make collector-build" >&2
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
    collector_log_path="${device_dir}/collector.log"
    simulator_pid_path="${run_dir}/simulator-${device_id}.pid"
    collector_pid_path="${run_dir}/collector-${device_id}.pid"

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

    "${collector_binary}" \
        --device-db-path "${db_path}" \
        --poll-interval-seconds "${COLLECTOR_POLL_INTERVAL_SECONDS}" \
        --redis-stream "${REDIS_STREAM}" \
        --redis-host "${REDIS_HOST}" \
        --redis-port "${REDIS_PORT}" \
        >"${collector_log_path}" 2>&1 &
    echo "$!" > "${collector_pid_path}"
    echo "started collector for device ${device_id}, pid=$(cat "${collector_pid_path}")"
done
