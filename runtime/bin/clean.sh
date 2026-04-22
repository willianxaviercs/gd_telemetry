#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap

if mountpoint -q "${devices_dir}"; then
    echo "runtime/devices is mounted; unmount it before cleaning" >&2
    exit 1
fi

if [[ ! -d "${devices_dir}" ]]; then
    echo "missing devices directory: ${devices_dir}" >&2
    exit 1
fi

find "${devices_dir}" -mindepth 1 -maxdepth 1 ! -name '.gitkeep' -exec rm -rf {} +

echo "cleaned generated device state under ${devices_dir}"
