#!/usr/bin/env bash

set -euo pipefail

source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/base.sh"

runtime_bootstrap

if [[ ! -d "${run_dir}" ]]; then
    echo "runtime is not running"
    exit 0
fi

stopped_any=0

for pid_file in "${run_dir}"/*.pid; do
    [[ -e "${pid_file}" ]] || break

    pid="$(cat "${pid_file}")"
    if [[ -n "${pid}" ]] && kill -0 "${pid}" 2>/dev/null; then
        kill "${pid}" 2>/dev/null || true
        stopped_any=1
    fi
done

for pid_file in "${run_dir}"/*.pid; do
    [[ -e "${pid_file}" ]] || break

    pid="$(cat "${pid_file}")"
    if [[ -z "${pid}" ]]; then
        rm -f "${pid_file}"
        continue
    fi

    for _ in {1..20}; do
        if ! kill -0 "${pid}" 2>/dev/null; then
            break
        fi
        sleep 0.1
    done

    if kill -0 "${pid}" 2>/dev/null; then
        kill -9 "${pid}" 2>/dev/null || true
    fi

    rm -f "${pid_file}"
done

if [[ ${stopped_any} -eq 1 ]]; then
    echo "stopped runtime processes"
else
    echo "runtime is not running"
fi
