#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap

schema_file="${repo_root}/schema/sqlite/device_events.sql"
runtime_require_topology

if [[ ! -f "${schema_file}" ]]; then
    echo "missing schema file: ${schema_file}" >&2
    exit 1
fi

runtime_require_command sqlite3

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"

mkdir -p "${devices_dir}"

for ((offset = 0; offset < DEVICE_COUNT; offset++)); do
    device_id=$((DEVICE_ID_START + offset))
    device_dir="${devices_dir}/${device_id}"
    db_path="${device_dir}/device.db"

    mkdir -p "${device_dir}"
    sqlite3 "${db_path}" < "${schema_file}"
done

echo "initialized ${DEVICE_COUNT} sqlite databases under ${devices_dir}"
