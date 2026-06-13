#!/usr/bin/env bash
#
# Build LogView for Windows.
#
# Two modes, auto-selected by host OS (override with WIN_MODE=cross|native):
#
#   cross  (default on Linux)
#       Cross-compiles with MinGW-w64. You need:
#         - a MinGW-w64 toolchain (x86_64-w64-mingw32-g++)
#         - a MinGW build of Qt6 pointed to by QT_ROOT or MXE_ROOT
#       Debian/Ubuntu toolchain: sudo apt install mingw-w64
#       Qt6 for MinGW is most easily obtained via MXE (https://mxe.cc) or
#       aqtinstall (`aqt install-qt linux desktop 6.x win64_mingw`).
#
#   native (default on Windows / Git-Bash / MSYS2)
#       Builds with the Qt + compiler already on PATH and runs windeployqt.
#
# Optional environment variables:
#   BUILD_TYPE        CMake build type (default: Release)
#   JOBS              Parallel build jobs (default: auto)
#   QT_ROOT           Path to the Qt6 install to use
#   MXE_ROOT          Path to an MXE checkout (e.g. /opt/mxe); implies cross mode
#   TOOLCHAIN_PREFIX  MinGW triple (default: x86_64-w64-mingw32)
#
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

BUILD_DIR="${PROJECT_ROOT}/build/windows"
OUT_DIR="${DIST_DIR}/windows"
WIN_MODE="${WIN_MODE:-}"
TOOLCHAIN_PREFIX="${TOOLCHAIN_PREFIX:-x86_64-w64-mingw32}"

# Decide mode if not forced.
if [ -z "${WIN_MODE}" ]; then
    if [ "${HOST_OS}" = "windows" ]; then WIN_MODE="native"; else WIN_MODE="cross"; fi
fi

require_cmd cmake "Install it with your package manager."
mkdir -p "${OUT_DIR}"

# ---------------------------------------------------------------------------
build_cross() {
    log "Cross-compiling LogView for Windows with MinGW-w64 (${BUILD_TYPE}, ${JOBS} jobs)"
    require_cmd "${TOOLCHAIN_PREFIX}-g++" "Install mingw-w64 (e.g. 'sudo apt install mingw-w64')."

    # Resolve the MinGW Qt6 prefix.
    local qt_prefix=""
    if [ -n "${QT_ROOT:-}" ]; then
        qt_prefix="${QT_ROOT}"
    elif [ -n "${MXE_ROOT:-}" ]; then
        qt_prefix="${MXE_ROOT}/usr/${TOOLCHAIN_PREFIX}.shared/qt6"
        [ -d "${qt_prefix}" ] || qt_prefix="${MXE_ROOT}/usr/${TOOLCHAIN_PREFIX}.static/qt6"
        # MXE ships its own toolchain triples; add its bin to PATH.
        export PATH="${MXE_ROOT}/usr/bin:${PATH}"
    fi
    [ -n "${qt_prefix}" ] || die "Set QT_ROOT (or MXE_ROOT) to a MinGW build of Qt6. See header comment."
    [ -d "${qt_prefix}" ] || die "Qt6 prefix not found: ${qt_prefix}"

    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/toolchain-mingw-w64.cmake" \
        -DTOOLCHAIN_PREFIX="${TOOLCHAIN_PREFIX}" \
        -DCMAKE_FIND_ROOT_PATH="${qt_prefix}" \
        -DCMAKE_PREFIX_PATH="${qt_prefix}"

    cmake --build "${BUILD_DIR}" -j "${JOBS}"

    local bin="${BUILD_DIR}/LogView.exe"
    [ -f "${bin}" ] || die "Build finished but LogView.exe not found at ${bin}"
    cp "${bin}" "${OUT_DIR}/"

    # Bundle Qt + runtime DLLs so the .exe is portable.
    log "Collecting dependent DLLs"
    if command -v "${TOOLCHAIN_PREFIX}-windeployqt6" >/dev/null 2>&1; then
        "${TOOLCHAIN_PREFIX}-windeployqt6" --release --dir "${OUT_DIR}" "${OUT_DIR}/LogView.exe" || \
            warn "windeployqt failed; copy Qt DLLs manually."
    else
        warn "No cross windeployqt found. Copy required Qt6*.dll and platform plugins"
        warn "from '${qt_prefix}/bin' and '${qt_prefix}/plugins/platforms' next to LogView.exe."
    fi
    ok "Windows binary -> ${OUT_DIR}/LogView.exe"
}

# ---------------------------------------------------------------------------
build_native() {
    log "Building LogView for Windows natively (${BUILD_TYPE}, ${JOBS} jobs)"

    local prefix_args=()
    if [ -n "${QT_ROOT:-}" ]; then
        prefix_args+=("-DCMAKE_PREFIX_PATH=${QT_ROOT}")
    elif [ -n "${CMAKE_PREFIX_PATH:-}" ]; then
        prefix_args+=("-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
    fi

    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        "${prefix_args[@]}"
    cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j "${JOBS}"

    # Locate the produced exe (single- vs multi-config generators differ).
    local bin
    bin="$(find "${BUILD_DIR}" -name LogView.exe -print -quit)"
    [ -n "${bin}" ] || die "Build finished but LogView.exe not found."
    cp "${bin}" "${OUT_DIR}/"

    if command -v windeployqt6 >/dev/null 2>&1; then
        windeployqt6 --release --dir "${OUT_DIR}" "${OUT_DIR}/LogView.exe"
    elif command -v windeployqt >/dev/null 2>&1; then
        windeployqt --release --dir "${OUT_DIR}" "${OUT_DIR}/LogView.exe"
    else
        warn "windeployqt not on PATH; add Qt's bin dir or deploy DLLs manually."
    fi
    ok "Windows binary -> ${OUT_DIR}/LogView.exe"
}

case "${WIN_MODE}" in
    cross)  build_cross ;;
    native) build_native ;;
    *)      die "Unknown WIN_MODE='${WIN_MODE}' (use 'cross' or 'native')." ;;
esac
