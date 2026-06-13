#!/usr/bin/env bash
#
# Shared helpers for the LogView build scripts.
# This file is meant to be sourced, not executed directly.

# Resolve important paths regardless of where the script is invoked from.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
DIST_DIR="${PROJECT_ROOT}/dist"

# ---------------------------------------------------------------------------
# Logging helpers
# ---------------------------------------------------------------------------
if [ -t 1 ]; then
    _C_RESET="\033[0m"; _C_BLUE="\033[1;34m"; _C_GREEN="\033[1;32m"
    _C_YELLOW="\033[1;33m"; _C_RED="\033[1;31m"
else
    _C_RESET=""; _C_BLUE=""; _C_GREEN=""; _C_YELLOW=""; _C_RED=""
fi

log()   { printf "${_C_BLUE}==>${_C_RESET} %s\n" "$*"; }
ok()    { printf "${_C_GREEN}OK${_C_RESET}  %s\n" "$*"; }
warn()  { printf "${_C_YELLOW}WARN${_C_RESET} %s\n" "$*" >&2; }
die()   { printf "${_C_RED}ERROR${_C_RESET} %s\n" "$*" >&2; exit 1; }

# ---------------------------------------------------------------------------
# Environment detection
# ---------------------------------------------------------------------------

# Number of parallel build jobs. Honors $JOBS if already set.
detect_jobs() {
    if [ -n "${JOBS:-}" ]; then echo "${JOBS}"; return; fi
    if command -v nproc >/dev/null 2>&1; then nproc
    elif command -v sysctl >/dev/null 2>&1; then sysctl -n hw.ncpu
    else echo 4; fi
}

# Host operating system: linux | macos | windows
detect_host_os() {
    case "$(uname -s)" in
        Linux*)              echo linux ;;
        Darwin*)             echo macos ;;
        MINGW*|MSYS*|CYGWIN*) echo windows ;;
        *)                   echo unknown ;;
    esac
}

require_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Required command '$1' not found in PATH. $2"
}

# Build type defaults to Release; override with $BUILD_TYPE.
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="$(detect_jobs)"
HOST_OS="$(detect_host_os)"
