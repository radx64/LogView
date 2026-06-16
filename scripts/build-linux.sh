#!/usr/bin/env bash
#
# Build LogView for Linux (native build).
#
# Requirements: cmake, a C++17 compiler (g++/clang++), make or ninja, and Qt6
# development packages (Core, Gui, Widgets).
#   Debian/Ubuntu: sudo apt install build-essential cmake qt6-base-dev
#   Fedora:        sudo dnf install gcc-c++ cmake qt6-qtbase-devel
#   Arch:          sudo pacman -S base-devel cmake qt6-base
#
# Optional environment variables:
#   BUILD_TYPE        CMake build type (default: Release)
#   JOBS              Parallel build jobs (default: auto)
#   CMAKE_PREFIX_PATH Extra path to a Qt6 install (e.g. Qt online installer)
#   QT_ROOT           Shorthand for a Qt6 prefix; appended to CMAKE_PREFIX_PATH
#   APPIMAGE=1        Also package an AppImage via linuxdeploy (if available)
#
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

BUILD_DIR="${PROJECT_ROOT}/build/linux"
OUT_DIR="${DIST_DIR}/linux"

log "Building LogView for Linux (${BUILD_TYPE}, ${JOBS} jobs)"

require_cmd cmake "Install it with your package manager."

# Assemble the Qt prefix path if the user pointed us at one.
PREFIX_ARGS=()
if [ -n "${QT_ROOT:-}" ]; then
    PREFIX_ARGS+=("-DCMAKE_PREFIX_PATH=${QT_ROOT}${CMAKE_PREFIX_PATH:+;${CMAKE_PREFIX_PATH}}")
elif [ -n "${CMAKE_PREFIX_PATH:-}" ]; then
    PREFIX_ARGS+=("-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
fi

# Prefer Ninja if present for faster builds.
GEN_ARGS=()
if command -v ninja >/dev/null 2>&1; then
    GEN_ARGS+=("-G" "Ninja")
fi

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    "${GEN_ARGS[@]}" \
    "${PREFIX_ARGS[@]}"

cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j "${JOBS}"

mkdir -p "${OUT_DIR}"
BIN="${BUILD_DIR}/LogView"
[ -f "${BIN}" ] || die "Build finished but binary not found at ${BIN}"
cp "${BIN}" "${OUT_DIR}/"
ok "Linux binary -> ${OUT_DIR}/LogView"

# Optional AppImage packaging.
if [ "${APPIMAGE:-0}" = "1" ]; then
    if command -v linuxdeploy >/dev/null 2>&1; then
        log "Packaging AppImage with linuxdeploy"
        DESKTOP_FILE="${PROJECT_ROOT}/packaging/LogView.desktop"
        ICON_FILE="${PROJECT_ROOT}/img/Gnome-Logviewer-32.png"
        [ -f "${DESKTOP_FILE}" ] || die "Missing desktop file: ${DESKTOP_FILE}"
        [ -f "${ICON_FILE}" ]    || die "Missing icon file: ${ICON_FILE}"

        # Let the Qt plugin find the same Qt we built against.
        export QMAKE="${QMAKE:-$(command -v qmake6 || command -v qmake || true)}"

        # linuxdeploy needs a desktop entry + icon to build an AppImage; the icon
        # is deployed as LogView.png so it matches the desktop's Icon=LogView.
        rm -f "${OUT_DIR}"/*.AppImage
        ( cd "${OUT_DIR}" && \
          linuxdeploy --appdir AppDir -e LogView \
              -d "${DESKTOP_FILE}" \
              -i "${ICON_FILE}" --icon-filename LogView \
              --plugin qt --output appimage ) \
          && ok "AppImage created in ${OUT_DIR}" \
          || die "AppImage packaging failed"
    else
        warn "APPIMAGE=1 set but 'linuxdeploy' not found; skipping packaging."
    fi
fi
