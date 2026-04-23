#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap

mkdir -p "${devices_dir}" "${run_dir}"

if mountpoint -q "${devices_dir}"; then
    echo "runtime/devices is mounted; unmount it before cleaning" >&2
    exit 1
fi

find "${devices_dir}" -mindepth 1 -maxdepth 1 ! -name '.gitkeep' -exec rm -rf {} +
find "${run_dir}" -mindepth 1 -maxdepth 1 ! -name '.gitkeep' -exec rm -rf {} +

echo "cleaned generated runtime state under ${runtime_dir}"
