#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd "${script_dir}/.." && pwd)"
devices_dir="${runtime_dir}/devices"
size="${1:-256m}"

mkdir -p "${devices_dir}"

if mountpoint -q "${devices_dir}"; then
    echo "runtime/devices is already mounted"
    exit 0
fi

sudo mount -t tmpfs -o "size=${size}" tmpfs "${devices_dir}"
echo "mounted tmpfs at ${devices_dir} with size ${size}"
