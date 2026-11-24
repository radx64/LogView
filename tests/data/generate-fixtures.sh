#!/usr/bin/env bash
#
# Regenerates the archive fixtures used by tests/ArchiveFormatsTest.cpp.
# Requires: tar, gzip, xz, zip. The generated files are committed so the tests
# run on Windows (no archive tools needed) exactly as on Linux.
#
# Payload layout (kept tiny and deterministic):
#   a.log, b.log   - two text logs, referenced by the tests
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")"

work="$(mktemp -d)"
trap 'rm -rf "${work}"' EXIT

mkdir -p "${work}/src/logs"
printf 'alpha 1\nalpha 2\nalpha 3\n' > "${work}/src/logs/a.log"
printf 'bravo 1\nbravo 2\n'          > "${work}/src/logs/b.log"
touch -d '2024-01-01 00:00:00 UTC' "${work}/src/logs/a.log" "${work}/src/logs/b.log"

rm -f ./*.gz ./*.xz ./*.zip ./*.tgz ./*.txz

tar_opts=(--sort=name --mtime='2024-01-01 00:00:00 UTC' --owner=0 --group=0 --numeric-owner)
make_tar() { tar "${tar_opts[@]}" -cf - -C "${work}/src" logs/a.log logs/b.log; }

# Single-stream compressed logs.
gzip -9 -n -c "${work}/src/logs/a.log" > single.log.gz
xz   -9 -c     "${work}/src/logs/a.log" > single.log.xz

# Multi-entry containers.
(cd "${work}/src" && zip -q -X -r "${OLDPWD}/multi.zip" logs)
make_tar | gzip -9 -n > multi.tar.gz
cp multi.tar.gz multi.tgz
make_tar | xz -9 > multi.tar.xz
cp multi.tar.xz multi.txz

# Nested: a zip holding a single-stream .xz/.gz, a tar.xz and a plain log.
mkdir -p "${work}/nest"
cp single.log.xz "${work}/nest/inner.log.xz"
cp single.log.gz "${work}/nest/inner.log.gz"
cp multi.tar.xz  "${work}/nest/inner.tar.xz"
cp "${work}/src/logs/b.log" "${work}/nest/top.log"
touch -d '2024-01-01 00:00:00 UTC' "${work}"/nest/*
(cd "${work}/nest" && zip -q -X -r "${OLDPWD}/nested.zip" .)

ls -l
