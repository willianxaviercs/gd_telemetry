#!/usr/bin/env bash

runtime_bootstrap() {
    local source_path

    source_path="${BASH_SOURCE[1]:-${BASH_SOURCE[0]}}"
    script_dir="$(cd "$(dirname "${source_path}")" && pwd)"
    runtime_dir="$(cd "${script_dir}/.." && pwd)"
    repo_root="$(cd "${runtime_dir}/.." && pwd)"
    devices_dir="${runtime_dir}/devices"
    run_dir="${runtime_dir}/run"
    topology_file="${runtime_dir}/topology/devices.env"
}

runtime_load_topology() {
    if [[ ! -f "${topology_file}" ]]; then
        echo "missing topology file: ${topology_file}" >&2
        exit 1
    fi

    # shellcheck disable=SC1090
    source "${topology_file}"
}

runtime_require_command() {
    local command_name

    command_name="${1}"
    if ! command -v "${command_name}" >/dev/null 2>&1; then
        echo "${command_name} is required" >&2
        exit 1
    fi
}
