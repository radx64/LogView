#!/usr/bin/env bash
#
# Build LogView for every target that is feasible from the current host.
#
#   - Linux:   built natively on Linux.
#   - Windows: cross-compiled from Linux (MinGW-w64) or natively on Windows.
#   - macOS:   built natively on macOS only (cannot cross-compile to macOS).
#
# Targets that cannot be built on this host are skipped with a warning instead
# of failing the whole run. Force a subset by passing target names:
#
#   ./build-all.sh                 # everything feasible on this host
#   ./build-all.sh linux windows   # only these targets
#
# All environment variables understood by the per-platform scripts
# (BUILD_TYPE, JOBS, QT_ROOT, MXE_ROOT, ...) are honored here too.
#
set -uo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

TARGETS=("$@")
if [ "${#TARGETS[@]}" -eq 0 ]; then
    TARGETS=(linux windows macos)
fi

declare -A RESULT

run_target() {
    local name="$1" script="$2"
    log "=== Target: ${name} ==="
    if bash "${SCRIPT_DIR}/${script}"; then
        RESULT["${name}"]="ok"
    else
        RESULT["${name}"]="FAILED"
    fi
}

for t in "${TARGETS[@]}"; do
    case "${t}" in
        linux)
            if [ "${HOST_OS}" = "linux" ]; then
                run_target linux build-linux.sh
            else
                warn "Skipping linux: requires a Linux host."; RESULT[linux]="skipped"
            fi
            ;;
        windows)
            if [ "${HOST_OS}" = "linux" ] || [ "${HOST_OS}" = "windows" ]; then
                run_target windows build-windows.sh
            else
                warn "Skipping windows: build from Linux (cross) or Windows (native)."
                RESULT[windows]="skipped"
            fi
            ;;
        macos)
            if [ "${HOST_OS}" = "macos" ]; then
                run_target macos build-macos.sh
            else
                warn "Skipping macos: must be built on a Mac."; RESULT[macos]="skipped"
            fi
            ;;
        *)
            warn "Unknown target '${t}' (expected: linux, windows, macos)."
            ;;
    esac
done

echo
log "Build summary:"
status=0
for t in "${TARGETS[@]}"; do
    r="${RESULT[${t}]:-skipped}"
    printf "  %-8s %s\n" "${t}" "${r}"
    [ "${r}" = "FAILED" ] && status=1
done
exit "${status}"
