#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd "${script_dir}/.." && pwd)"
topology_file="${runtime_dir}/topology/devices.env"
devices_dir="${runtime_dir}/devices"

if [[ ! -f "${topology_file}" ]]; then
    echo "missing topology file: ${topology_file}" >&2
    exit 1
fi

# shellcheck disable=SC1090
source "${topology_file}"

: "${DEVICE_ID_START:?DEVICE_ID_START is required}"
: "${DEVICE_COUNT:?DEVICE_COUNT is required}"

mkdir -p "${devices_dir}"

for ((offset = 0; offset < DEVICE_COUNT; offset++)); do
    device_id=$((DEVICE_ID_START + offset))
    device_dir="${devices_dir}/${device_id}"

    mkdir -p "${device_dir}"
done

echo "created ${DEVICE_COUNT} device directories under ${devices_dir}"
