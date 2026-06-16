#!/usr/bin/env bash
#
# Build LogView for Windows.
#
# Two modes, auto-selected by host OS (override with WIN_MODE=cross|native):
#
#   cross  (default on Linux)
#       Cross-compiles with MinGW-w64. You need:
#         - a MinGW-w64 toolchain (sudo apt install mingw-w64) - the POSIX-thread
#           variant is used by default to match Qt's official ABI
#         - a MinGW build of Qt6 (the *target* libraries), pointed to by QT_ROOT
#         - a *host* Qt6 of the SAME version for the code-gen tools (moc/rcc/uic),
#           because the Windows package ships Windows .exe tools. On Debian/Ubuntu
#           the system Qt6 (qt6-base-dev) is auto-detected as QT_HOST_PATH.
#
#       Linkage: by default the build prefers a *static* Qt when one is available
#       (e.g. MXE's x86_64-w64-mingw32.static target), producing a single
#       self-contained LogView.exe with no DLLs. With a shared Qt it folds the
#       C++ runtime in and bundles the Qt DLLs/plugins. Force a choice with
#       QT_LINKAGE=static|shared (default: auto).
#
#       Fully static example (MXE):
#         MXE_ROOT=/opt/mxe ./scripts/build-windows.sh
#
#       Getting the MinGW Qt6 (must match your host Qt6 version), e.g. 6.9.2:
#         pip install aqtinstall
#         aqt install-qt linux desktop 6.9.2 win64_mingw -O ~/Qt
#         QT_ROOT=~/Qt/6.9.2/mingw_64 ./scripts/build-windows.sh
#       (MXE via MXE_ROOT is also supported.)
#
#   native (default on Windows / Git-Bash / MSYS2)
#       Builds with the Qt + compiler already on PATH and runs windeployqt.
#
# Optional environment variables:
#   BUILD_TYPE            CMake build type (default: Release)
#   JOBS                  Parallel build jobs (default: auto)
#   QT_LINKAGE            Qt linkage preference: auto (default) | static | shared
#   QT_ROOT               Path to the (target) MinGW Qt6 install
#   QT_HOST_PATH          Host Qt6 prefix for moc/rcc/uic (auto on Debian/Ubuntu)
#   QT_HOST_PATH_CMAKE_DIR  Host Qt6 cmake dir (auto on Debian/Ubuntu multiarch)
#   MXE_ROOT              Path to an MXE checkout (e.g. /opt/mxe)
#   TOOLCHAIN_PREFIX      MinGW triple (default: x86_64-w64-mingw32)
#   MINGW_THREAD_SUFFIX   "-posix" (default) or "" for the win32-thread compiler
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

# MinGW thread model. Qt's official MinGW builds use POSIX threads, so we use
# the Debian "-posix" alternative by default. Override with "" for win32.
MINGW_THREAD_SUFFIX="${MINGW_THREAD_SUFFIX:--posix}"
OBJDUMP="${TOOLCHAIN_PREFIX}-objdump"

# Windows DLLs that ship with the OS and must NOT be bundled.
_is_system_dll() {
    case "${1,,}" in
        api-ms-*|ext-ms-*|kernel32.dll|user32.dll|gdi32.dll|shell32.dll|\
        shcore.dll|shlwapi.dll|ole32.dll|advapi32.dll|ws2_32.dll|msvcrt.dll|\
        version.dll|winmm.dll|userenv.dll|netapi32.dll|uxtheme.dll|dwmapi.dll|\
        imm32.dll|oleaut32.dll|comdlg32.dll|rpcrt4.dll|crypt32.dll|secur32.dll|\
        d3d*.dll|dxgi.dll|dwrite.dll|mpr.dll|authz.dll|bcrypt.dll|wtsapi32.dll|\
        setupapi.dll|winspool.drv|ntdll.dll|gdiplus.dll|opengl32.dll|glu32.dll|\
        comctl32.dll) return 0 ;;
        *) return 1 ;;
    esac
}

# Recursively copy non-system DLL dependencies of $1 into ${OUT_DIR}.
# Search paths are provided via the WIN_DLL_PATHS array.
_collect_dlls() {
    local bin="$1" name src
    while read -r _ _ name; do
        [ -n "${name}" ] || continue
        _is_system_dll "${name}" && continue
        [ -f "${OUT_DIR}/${name}" ] && continue
        src=""
        for d in "${WIN_DLL_PATHS[@]}"; do
            [ -f "${d}/${name}" ] && { src="${d}/${name}"; break; }
        done
        if [ -n "${src}" ]; then
            cp "${src}" "${OUT_DIR}/"; echo "  + ${name}"
            _collect_dlls "${OUT_DIR}/${name}"
        else
            warn "  dependency not found (assuming system): ${name}"
        fi
    done < <("${OBJDUMP}" -p "${bin}" 2>/dev/null | grep "DLL Name:")
}

# ---------------------------------------------------------------------------
build_cross() {
    log "Cross-compiling LogView for Windows with MinGW-w64 (${BUILD_TYPE}, ${JOBS} jobs)"

    # Preferred Qt linkage: auto (prefer static), static, or shared.
    local qt_linkage="${QT_LINKAGE:-auto}"

    # Resolve the MinGW (target) Qt6 prefix. MXE ships separate static/shared
    # trees (and a matching compiler per tree), so pick one based on QT_LINKAGE.
    local qt_prefix=""
    local toolchain_prefix="${TOOLCHAIN_PREFIX}"
    local thread_suffix="${MINGW_THREAD_SUFFIX}"
    local mxe_sysroot=""
    if [ -n "${QT_ROOT:-}" ]; then
        qt_prefix="${QT_ROOT}"
    elif [ -n "${MXE_ROOT:-}" ]; then
        export PATH="${MXE_ROOT}/usr/bin:${PATH}"
        local mxe_static="${MXE_ROOT}/usr/${TOOLCHAIN_PREFIX}.static/qt6"
        local mxe_shared="${MXE_ROOT}/usr/${TOOLCHAIN_PREFIX}.shared/qt6"
        case "${qt_linkage}" in
            shared) qt_prefix="${mxe_shared}" ;;
            static) qt_prefix="${mxe_static}" ;;
            *)      # auto: prefer static, fall back to shared
                    if [ -d "${mxe_static}" ]; then qt_prefix="${mxe_static}"
                    else qt_prefix="${mxe_shared}"; fi ;;
        esac
        # MXE provides its own compiler per linkage (suffixed .static/.shared)
        # and does not use the Debian "-posix" thread variant.
        if [ "${qt_prefix}" = "${mxe_static}" ]; then
            toolchain_prefix="${TOOLCHAIN_PREFIX}.static"
        else
            toolchain_prefix="${TOOLCHAIN_PREFIX}.shared"
        fi
        thread_suffix=""
        mxe_sysroot="${MXE_ROOT}/usr/${toolchain_prefix}"
    else
        # Fall back to a default aqtinstall layout: ~/Qt/<ver>/mingw_64
        qt_prefix="$(ls -d "${HOME}"/Qt/*/mingw_64 2>/dev/null | sort -V | tail -n1 || true)"
    fi
    [ -n "${qt_prefix}" ] || die "Set QT_ROOT to a MinGW build of Qt6 (see header comment)."
    [ -d "${qt_prefix}" ] || die "Qt6 prefix not found: ${qt_prefix}"

    # Detect static vs shared Qt from what is actually on disk: a static Qt has
    # libQt6Core.a and no Qt6Core.dll. An explicit QT_LINKAGE wins.
    local qt_is_static=0
    if [ -f "${qt_prefix}/lib/libQt6Core.a" ] && \
       ! ls "${qt_prefix}"/bin/Qt6Core.dll >/dev/null 2>&1; then
        qt_is_static=1
    fi
    case "${qt_linkage}" in
        static) qt_is_static=1 ;;
        shared) qt_is_static=0 ;;
    esac

    local cxx="${toolchain_prefix}-g++${thread_suffix}"
    require_cmd "${cxx}" "Install mingw-w64 (e.g. 'sudo apt install mingw-w64') or set MXE_ROOT/QT_ROOT."
    OBJDUMP="${toolchain_prefix}-objdump"

    if [ "${qt_is_static}" = "1" ]; then
        log "Target Qt6 (MinGW, static): ${qt_prefix}"
    else
        log "Target Qt6 (MinGW, shared): ${qt_prefix}"
    fi

    # The Windows Qt ships Windows .exe tools (moc/rcc/uic) that can't run on a
    # Linux host, so point Qt at a matching host Qt for codegen via QT_HOST_PATH.
    local host_args=()
    if [ -n "${QT_HOST_PATH:-}" ]; then
        host_args+=("-DQT_HOST_PATH=${QT_HOST_PATH}")
        [ -n "${QT_HOST_PATH_CMAKE_DIR:-}" ] && \
            host_args+=("-DQT_HOST_PATH_CMAKE_DIR=${QT_HOST_PATH_CMAKE_DIR}")
    elif [ "${HOST_OS}" = "linux" ]; then
        # Autodetect a system (Debian/Ubuntu multiarch) host Qt6.
        local host_cmake
        host_cmake="$(ls -d /usr/lib/*/cmake/Qt6 2>/dev/null | head -n1 || true)"
        if [ -n "${host_cmake}" ]; then
            # QT_HOST_PATH_CMAKE_DIR is the dir that *contains* the Qt6/ config
            # dir (e.g. /usr/lib/x86_64-linux-gnu/cmake).
            host_args+=("-DQT_HOST_PATH=/usr"
                        "-DQT_HOST_PATH_CMAKE_DIR=$(dirname "${host_cmake}")")
            log "Host Qt6 tools: ${host_cmake}"
        else
            warn "No system host Qt6 found; set QT_HOST_PATH to a host Qt matching ${qt_prefix##*/}."
        fi
    fi

    # Static Qt pulls in many transitive deps (png, harfbuzz, zlib, ...) that
    # live in the MXE sysroot, so make CMake search there too.
    local find_root="${qt_prefix}"
    [ -n "${mxe_sysroot}" ] && find_root="${mxe_sysroot};${qt_prefix}"

    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/toolchain-mingw-w64.cmake" \
        -DTOOLCHAIN_PREFIX="${toolchain_prefix}" \
        -DMINGW_THREAD_SUFFIX="${thread_suffix}" \
        -DCMAKE_FIND_ROOT_PATH="${find_root}" \
        -DCMAKE_PREFIX_PATH="${qt_prefix}" \
        -DLOGVIEW_STATIC=ON \
        "${host_args[@]}"

    cmake --build "${BUILD_DIR}" -j "${JOBS}"

    local bin="${BUILD_DIR}/LogView.exe"
    [ -f "${bin}" ] || die "Build finished but LogView.exe not found at ${bin}"
    cp "${bin}" "${OUT_DIR}/"

    # A static Qt build bakes Qt, its plugins and the runtime into the .exe, so
    # there is nothing to bundle - ship the single self-contained executable.
    if [ "${qt_is_static}" = "1" ]; then
        ok "Static Windows binary -> ${OUT_DIR}/LogView.exe (self-contained, no DLLs)"
        return
    fi

    # Shared Qt: the compiler runtime is folded in by LOGVIEW_STATIC, but Qt and
    # its plugins still ship as DLLs. windeployqt is a Windows binary and can't
    # run on a Linux host, so collect dependencies manually.
    log "Collecting dependent DLLs"
    local gcc_libdir
    gcc_libdir="$(ls -d /usr/lib/gcc/${toolchain_prefix}/*${thread_suffix} 2>/dev/null | sort -V | tail -n1 || true)"
    WIN_DLL_PATHS=(
        "${qt_prefix}/bin"
        "${gcc_libdir}"
        "/usr/${toolchain_prefix}/lib"
        "/usr/${toolchain_prefix}/bin"
    )
    _collect_dlls "${OUT_DIR}/LogView.exe"

    # The Windows platform plugin is mandatory for any GUI app.
    log "Bundling Qt plugins"
    mkdir -p "${OUT_DIR}/platforms"
    cp "${qt_prefix}/plugins/platforms/qwindows.dll" "${OUT_DIR}/platforms/" \
        && _collect_dlls "${OUT_DIR}/platforms/qwindows.dll"
    if [ -f "${qt_prefix}/plugins/styles/qmodernwindowsstyle.dll" ]; then
        mkdir -p "${OUT_DIR}/styles"
        cp "${qt_prefix}/plugins/styles/qmodernwindowsstyle.dll" "${OUT_DIR}/styles/" \
            && _collect_dlls "${OUT_DIR}/styles/qmodernwindowsstyle.dll"
    fi

    ok "Windows bundle -> ${OUT_DIR}/ (run LogView.exe)"
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
