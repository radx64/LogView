#!/usr/bin/env bash
#
# Build LogView for macOS (native build — must run on macOS).
#
# Cross-compiling to macOS from Linux/Windows requires the Apple SDK and is not
# supported by this script. Run this on a Mac (or macOS CI runner).
#
# Requirements: Xcode command line tools, cmake, and Qt6.
#   brew install cmake qt6
#
# Optional environment variables:
#   BUILD_TYPE        CMake build type (default: Release)
#   JOBS              Parallel build jobs (default: auto)
#   QT_ROOT           Path to a Qt6 install (default: autodetected via brew)
#   MACOS_ARCH        Target arch(s): "arm64", "x86_64", or "arm64;x86_64"
#                     (universal). Default: native arch.
#
set -euo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/lib.sh"

[ "${HOST_OS}" = "macos" ] || die "build-macos.sh must run on macOS (cannot cross-compile to macOS)."

BUILD_DIR="${PROJECT_ROOT}/build/macos"
OUT_DIR="${DIST_DIR}/macos"

log "Building LogView for macOS (${BUILD_TYPE}, ${JOBS} jobs)"
require_cmd cmake "Install it with 'brew install cmake'."

# Locate Qt6: explicit QT_ROOT wins, otherwise ask Homebrew.
QT_PREFIX="${QT_ROOT:-}"
if [ -z "${QT_PREFIX}" ] && command -v brew >/dev/null 2>&1; then
    QT_PREFIX="$(brew --prefix qt6 2>/dev/null || brew --prefix qt 2>/dev/null || true)"
fi
[ -n "${QT_PREFIX}" ] || die "Could not find Qt6. Install it ('brew install qt6') or set QT_ROOT."

ARCH_ARGS=()
if [ -n "${MACOS_ARCH:-}" ]; then
    ARCH_ARGS+=("-DCMAKE_OSX_ARCHITECTURES=${MACOS_ARCH}")
fi

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_PREFIX_PATH="${QT_PREFIX}" \
    "${ARCH_ARGS[@]}"

cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j "${JOBS}"

mkdir -p "${OUT_DIR}"

# The CMake config produces a plain executable named "LogView". Package it into
# a minimal .app bundle and deploy Qt frameworks with macdeployqt.
APP="${OUT_DIR}/LogView.app"
rm -rf "${APP}"
mkdir -p "${APP}/Contents/MacOS" "${APP}/Contents/Resources"

BIN="${BUILD_DIR}/LogView"
[ -f "${BIN}" ] || die "Build finished but binary not found at ${BIN}"
cp "${BIN}" "${APP}/Contents/MacOS/LogView"

cat > "${APP}/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key><string>LogView</string>
    <key>CFBundleIdentifier</key><string>org.logview.LogView</string>
    <key>CFBundleName</key><string>LogView</string>
    <key>CFBundlePackageType</key><string>APPL</string>
    <key>CFBundleShortVersionString</key><string>0.0.3</string>
    <key>NSHighResolutionCapable</key><true/>
</dict>
</plist>
PLIST

if command -v macdeployqt6 >/dev/null 2>&1; then
    macdeployqt6 "${APP}" || warn "macdeployqt6 reported issues."
elif [ -x "${QT_PREFIX}/bin/macdeployqt" ]; then
    "${QT_PREFIX}/bin/macdeployqt" "${APP}" || warn "macdeployqt reported issues."
elif command -v macdeployqt >/dev/null 2>&1; then
    macdeployqt "${APP}" || warn "macdeployqt reported issues."
else
    warn "macdeployqt not found; the .app will depend on your local Qt install."
fi

ok "macOS app bundle -> ${APP}"
