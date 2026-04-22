#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap

collector_binary="${repo_root}/collector/build/collector"
collector_build_script="${repo_root}/collector/build.sh"
collector_cmake_file="${repo_root}/collector/CMakeLists.txt"
collector_source_file="${repo_root}/collector/src/main.cpp"

runtime_require_topology
runtime_require_command bash

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"
: "${COLLECTOR_POLL_INTERVAL_SECONDS:?COLLECTOR_POLL_INTERVAL_SECONDS is required}"
: "${REDIS_HOST:?REDIS_HOST is required}"
: "${REDIS_PORT:?REDIS_PORT is required}"
: "${REDIS_STREAM:?REDIS_STREAM is required}"

if [[ ! -f "${collector_cmake_file}" ]]; then
    echo "missing collector build file: ${collector_cmake_file}" >&2
    exit 1
fi

if [[ ! -f "${collector_source_file}" ]]; then
    echo "missing collector source file: ${collector_source_file}" >&2
    exit 1
fi

if [[ ! -x "${collector_binary}" || "${collector_binary}" -ot "${collector_build_script}" || "${collector_binary}" -ot "${collector_cmake_file}" || "${collector_binary}" -ot "${collector_source_file}" ]]; then
    if [[ ! -x "${collector_build_script}" ]]; then
        echo "missing collector binary and build script is not executable: ${collector_build_script}" >&2
        exit 1
    fi

    (
        cd "${repo_root}/collector"
        exec "./$(basename "${collector_build_script}")"
    )
fi

if [[ ! -x "${collector_binary}" ]]; then
    echo "collector binary is missing after build: ${collector_binary}" >&2
    exit 1
fi

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
    device_dir="${devices_dir}/${device_id}"
    db_path="${device_dir}/device.db"
    log_path="${device_dir}/collector.log"

    if [[ ! -f "${db_path}" ]]; then
        echo "missing sqlite database for device ${device_id}: ${db_path}" >&2
        exit 1
    fi

    (
        exec "${collector_binary}" \
            --device-db-path "${db_path}" \
            --poll-interval-seconds "${COLLECTOR_POLL_INTERVAL_SECONDS}" \
            --redis-stream "${REDIS_STREAM}" \
            --redis-host "${REDIS_HOST}" \
            --redis-port "${REDIS_PORT}"
    ) >"${log_path}" 2>&1 &

    pids+=("$!")
    echo "started collector for device ${device_id}, log=${log_path}, pid=${pids[-1]}"
done

wait -n "${pids[@]}"
