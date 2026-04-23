#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap

if ! mountpoint -q "${devices_dir}"; then
    echo "runtime/devices is not mounted"
    exit 0
fi

sudo umount "${devices_dir}"
echo "unmounted ${devices_dir}"
