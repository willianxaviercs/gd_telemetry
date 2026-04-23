#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap
size="${1:-256m}"

mkdir -p "${devices_dir}"

if mountpoint -q "${devices_dir}"; then
    echo "runtime/devices is already mounted"
    exit 0
fi

sudo mount -t tmpfs -o "size=${size}" tmpfs "${devices_dir}"
echo "mounted tmpfs at ${devices_dir} with size ${size}"
