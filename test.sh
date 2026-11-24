#!/usr/bin/env bash
#
# Build and run the LogView unit tests in the current environment.
#
# Usage:
#   ./test.sh                  # configure (if needed), build tests, run them
#   BUILD_TYPE=Debug ./test.sh
#   JOBS=4 ./test.sh
#   ./test.sh clean            # remove the build directory first
#   ./test.sh -R Archive       # extra arguments are passed to ctest
#
# Optional environment variables:
#   BUILD_DIR         Build directory (default: ./build, shared with build.sh)
#   CMAKE_EXTRA_ARGS  Extra arguments for the cmake configure step
#
# Works on Linux and on Windows (Git Bash / MSYS2) with Qt + a compiler on PATH.
#
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

if ! command -v cmake >/dev/null 2>&1; then
    echo "error: cmake not found on PATH" >&2
    exit 1
fi

if [ "${1:-}" = "clean" ]; then
    echo ">> Removing ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
    shift
fi

echo ">> Configuring (${BUILD_TYPE})"
# shellcheck disable=SC2086
cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DLOGVIEW_BUILD_TESTS=ON \
    ${CMAKE_EXTRA_ARGS:-}

echo ">> Building tests (${JOBS} jobs)"
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --target LogViewTests -j "${JOBS}"

echo ">> Running tests"
ctest --test-dir "${BUILD_DIR}" -C "${BUILD_TYPE}" --output-on-failure "$@"
