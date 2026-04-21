#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
runtime_dir="${repo_root}/runtime"
topology_file="${runtime_dir}/topology/devices.env"
devices_dir="${runtime_dir}/devices"

if [[ ! -f "${topology_file}" ]]; then
    echo "missing topology file: ${topology_file}" >&2
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "python3 is required" >&2
    exit 1
fi

# shellcheck disable=SC1090
source "${topology_file}"

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"
: "${SIMULATOR_INTERVAL_SECONDS:?SIMULATOR_INTERVAL_SECONDS is required}"

pids=()

cleanup() {
    local pid

    for pid in "${pids[@]:-}"; do
        if kill -0 "${pid}" 2>/dev/null; then
            kill "${pid}" 2>/dev/null || true
        fi
    done

    for pid in "${pids[@]:-}"; do
        wait "${pid}" 2>/dev/null || true
    done
}

trap cleanup EXIT INT TERM

for ((offset = 0; offset < DEVICE_COUNT; offset++)); do
    device_id=$((DEVICE_ID_START + offset))
    device_dir="${devices_dir}/device-${device_id}"
    db_path="${device_dir}/device.db"
    log_path="${device_dir}/simulator.log"

    if [[ ! -f "${db_path}" ]]; then
        echo "missing sqlite database for device ${device_id}: ${db_path}" >&2
        exit 1
    fi

    (
        exec python3 "${repo_root}/simulator/main.py" \
            --device-id "${device_id}" \
            --interval-seconds "${SIMULATOR_INTERVAL_SECONDS}"
    ) >"${log_path}" 2>&1 &

    pids+=("$!")
    echo "started simulator for device ${device_id}, log=${log_path}, pid=${pids[-1]}"
done

wait -n "${pids[@]}"
