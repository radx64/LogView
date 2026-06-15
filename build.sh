#!/usr/bin/env bash
#
# Build the LogView binary.
#
# Usage:
#   ./build.sh                 # configure (if needed) and build
#   BUILD_TYPE=Debug ./build.sh
#   JOBS=4 ./build.sh
#   ./build.sh clean           # remove the build directory first
#
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

if ! command -v cmake >/dev/null 2>&1; then
    echo "error: cmake not found on PATH" >&2
    exit 1
fi

if [ "${1:-}" = "clean" ]; then
    echo ">> Removing ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
fi

echo ">> Configuring (${BUILD_TYPE})"
cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo ">> Building (${JOBS} jobs)"
cmake --build "${BUILD_DIR}" -j "${JOBS}"

echo ">> Done: ${BUILD_DIR}/LogView"
