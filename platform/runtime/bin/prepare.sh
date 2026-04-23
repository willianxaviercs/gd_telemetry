#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap
runtime_load_topology
runtime_require_command sqlite3

schema_file="${repo_root}/schema/sqlite/device_events.sql"

if [[ ! -f "${schema_file}" ]]; then
    echo "missing schema file: ${schema_file}" >&2
    exit 1
fi

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"

mkdir -p "${devices_dir}" "${run_dir}"

for pid_file in "${run_dir}"/*.pid; do
    [[ -e "${pid_file}" ]] || break

    pid="$(cat "${pid_file}")"
    if [[ -n "${pid}" ]] && kill -0 "${pid}" 2>/dev/null; then
        echo "runtime already running; stop it first with: make runtime-stop" >&2
        exit 1
    fi
done

find "${devices_dir}" -mindepth 1 -maxdepth 1 ! -name '.gitkeep' -exec rm -rf {} +
find "${run_dir}" -mindepth 1 -maxdepth 1 ! -name '.gitkeep' -exec rm -rf {} +

for ((offset = 0; offset < DEVICE_COUNT; offset++)); do
    device_id=$((DEVICE_ID_START + offset))
    device_dir="${devices_dir}/${device_id}"
    db_path="${device_dir}/device.db"

    mkdir -p "${device_dir}"
    sqlite3 "${db_path}" < "${schema_file}"
done

echo "prepared ${DEVICE_COUNT} device databases under ${devices_dir}"
