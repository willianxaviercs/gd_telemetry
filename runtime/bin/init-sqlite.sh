#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/../.." && pwd)"
runtime_dir="${repo_root}/runtime"
topology_file="${runtime_dir}/topology/devices.env"
schema_file="${repo_root}/schema/sqlite/device_events.sql"
devices_dir="${runtime_dir}/devices"

if [[ ! -f "${topology_file}" ]]; then
    echo "missing topology file: ${topology_file}" >&2
    exit 1
fi

if [[ ! -f "${schema_file}" ]]; then
    echo "missing schema file: ${schema_file}" >&2
    exit 1
fi

if ! command -v sqlite3 >/dev/null 2>&1; then
    echo "sqlite3 is required" >&2
    exit 1
fi

# shellcheck disable=SC1090
source "${topology_file}"

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"

mkdir -p "${devices_dir}"

for ((offset = 0; offset < DEVICE_COUNT; offset++)); do
    device_id=$((DEVICE_ID_START + offset))
    device_dir="${devices_dir}/device-${device_id}"
    db_path="${device_dir}/device.db"

    mkdir -p "${device_dir}"
    sqlite3 "${db_path}" < "${schema_file}"
done

echo "initialized ${DEVICE_COUNT} sqlite databases under ${devices_dir}"
