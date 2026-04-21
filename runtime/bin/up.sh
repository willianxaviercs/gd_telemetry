#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd "${script_dir}/.." && pwd)"
devices_dir="${runtime_dir}/devices"
mount_script="${script_dir}/mount-tmpfs.sh"
create_script="${script_dir}/create-devices.sh"
init_script="${script_dir}/init-sqlite.sh"
run_simulators_script="${script_dir}/run-simulators.sh"
clean_script="${script_dir}/clean.sh"
unmount_script="${script_dir}/unmount-tmpfs.sh"

mounted_by_up=0
skip_tmpfs_mount="${RUNTIME_SKIP_TMPFS_MOUNT:-0}"
cleaned_up=0

cleanup() {
    if [[ ${cleaned_up} -eq 1 ]]; then
        return
    fi

    cleaned_up=1

    if [[ ${mounted_by_up} -eq 1 && -d "${devices_dir}" ]]; then
        "${unmount_script}" || true
    fi

    if [[ ! -d "${devices_dir}" ]] || ! mountpoint -q "${devices_dir}"; then
        "${clean_script}" || true
    fi
}

trap cleanup EXIT INT TERM

if [[ "${skip_tmpfs_mount}" != "1" ]] && ! mountpoint -q "${devices_dir}"; then
    "${mount_script}"
    mounted_by_up=1
fi

"${create_script}"
"${init_script}"
"${run_simulators_script}"
