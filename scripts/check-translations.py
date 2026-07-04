#!/usr/bin/env python3
"""Validate LogView translation catalogs against the keys used in the code.

The UI looks up strings by semantic keys passed as string literals to
``Lang::tr("...")`` (see src/Translator.hpp). Because the keys live in the
source, the code itself is the source of truth for *which* keys are required.

This script:
  1. Scans src/ for ``Lang::tr("...")`` and collects every key used in code.
  2. Parses every lang/*.lang catalog (same rules as the C++ loader).
  3. Reports problems and exits non-zero if any *error* is found:
       - a key used in code but missing from en.lang         (ERROR)
       - a key used in code but missing from another catalog (ERROR)
       - required metadata key missing from a catalog        (ERROR)
       - a duplicated key within a catalog                    (ERROR)
       - a catalog key that is not present in en.lang         (warning)
       - an en.lang key that is never used in code            (warning)

Limitation: only *literal* keys can be detected. A key assembled at runtime
(e.g. "menu." + name) is invisible to a static scan; keep keys as literals.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_DIR = REPO_ROOT / "src"
LANG_DIR = REPO_ROOT / "lang"

ENGLISH_CODE = "en"
SOURCE_SUFFIXES = {".cpp", ".hpp", ".h", ".cc", ".cxx"}

# Keys read directly from the catalog (not via Lang::tr) that must still exist
# in every catalog. See Translator::displayName / loadCatalog.
REQUIRED_METADATA_KEYS = {"language.name"}

# Matches Lang::tr("key") and Lang::tr(QStringLiteral("key")).
KEY_USAGE_RE = re.compile(
    r'Lang::tr\s*\(\s*(?:QStringLiteral\s*\(\s*)?"([^"]+)"'
)


def collect_used_keys() -> set[str]:
    """Return the set of translation keys referenced from the source tree."""
    keys: set[str] = set()
    for path in sorted(SRC_DIR.rglob("*")):
        if path.suffix.lower() not in SOURCE_SUFFIXES or not path.is_file():
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        keys.update(KEY_USAGE_RE.findall(text))
    return keys


def parse_catalog(path: Path) -> tuple[dict[str, str], list[str]]:
    """Parse a .lang file.

    Returns (key->value, duplicate_messages). A key that appears on more than
    one line is reported once per extra occurrence, with both line numbers.
    """
    catalog: dict[str, str] = {}
    first_line: dict[str, int] = {}
    duplicates: list[str] = []
    for line_number, raw_line in enumerate(
            path.read_text(encoding="utf-8").splitlines(), start=1):
        if not raw_line or raw_line.startswith("#"):
            continue
        if "=" not in raw_line:
            continue
        key, value = raw_line.split("=", 1)
        key = key.strip()
        if not key:
            continue
        if key in catalog:
            duplicates.append(
                f"duplicated key '{key}' (line {line_number}, "
                f"first defined on line {first_line[key]})")
        else:
            first_line[key] = line_number
        catalog[key] = value
    return catalog, duplicates


def discover_catalogs() -> dict[str, Path]:
    """Map language code -> path for every lang/*.lang file."""
    return {p.stem: p for p in sorted(LANG_DIR.glob("*.lang"))}


def main() -> int:
    errors: list[str] = []
    warnings: list[str] = []

    catalogs = discover_catalogs()
    if ENGLISH_CODE not in catalogs:
        print(f"error: reference catalog {LANG_DIR}/{ENGLISH_CODE}.lang not found",
              file=sys.stderr)
        return 1

    used_keys = collect_used_keys()
    if not used_keys:
        print("error: no Lang::tr(\"...\") usages found; is the scan path correct?",
              file=sys.stderr)
        return 1

    parsed: dict[str, dict[str, str]] = {}
    for code, path in catalogs.items():
        catalog, duplicates = parse_catalog(path)
        parsed[code] = catalog
        for dup in duplicates:
            errors.append(f"{code}.lang: {dup}")

    english_keys = set(parsed[ENGLISH_CODE])

    # 1) Every key used in code must exist in English (catches new strings).
    for key in sorted(used_keys - english_keys):
        errors.append(f"en.lang: key '{key}' is used in code but missing "
                      f"(new string not added to English?)")

    # 2) English keys never referenced from code (possible dead entries).
    for key in sorted(english_keys - used_keys - REQUIRED_METADATA_KEYS):
        warnings.append(f"en.lang: key '{key}' is not used anywhere in code")

    # 3) Per-catalog checks.
    required_keys = used_keys | REQUIRED_METADATA_KEYS
    for code in sorted(catalogs):
        keys = set(parsed[code])

        for key in sorted(required_keys - keys):
            errors.append(f"{code}.lang: missing translation for key '{key}'")

        # Keys present here but unknown to English are stale/renamed.
        for key in sorted(keys - english_keys - REQUIRED_METADATA_KEYS):
            warnings.append(f"{code}.lang: key '{key}' is not present in en.lang "
                            f"(stale or renamed?)")

    # Report.
    print(f"Scanned {len(used_keys)} keys used in code across {SRC_DIR}")
    print(f"Catalogs: {', '.join(f'{c} ({len(parsed[c])})' for c in sorted(catalogs))}")
    print()

    for warning in warnings:
        print(f"warning: {warning}")
    for error in errors:
        print(f"error: {error}")

    print()
    if errors:
        print(f"FAILED: {len(errors)} error(s), {len(warnings)} warning(s)")
        return 1
    print(f"OK: all translations present ({len(warnings)} warning(s))")
    return 0


if __name__ == "__main__":
    sys.exit(main())
