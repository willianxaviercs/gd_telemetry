#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd "${script_dir}/.." && pwd)"
devices_dir="${runtime_dir}/devices"

if ! mountpoint -q "${devices_dir}"; then
    echo "runtime/devices is not mounted"
    exit 0
fi

sudo umount "${devices_dir}"
echo "unmounted ${devices_dir}"
