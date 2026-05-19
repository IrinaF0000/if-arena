#!/usr/bin/env python3
"""Check forbidden include dependencies between project architecture layers."""

from __future__ import annotations

import re
import sys
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SRC_ROOT = PROJECT_ROOT / "src"
SOURCE_EXTENSIONS = {".cpp", ".hpp", ".h", ".hh", ".cxx", ".cc"}
INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]')

FORBIDDEN_INCLUDES = {
    "Core": {"Features", "App", "IO"},
    "Features": {"App"},
    "IO": {"App"},
}


def source_layer(path: Path) -> str | None:
    try:
        relative = path.relative_to(SRC_ROOT)
    except ValueError:
        return None
    return relative.parts[0] if relative.parts else None


def include_layer(include: str, source: Path) -> str | None:
    normalized = include.replace("\\", "/")

    if normalized.startswith("../") or normalized.startswith("./"):
        resolved = (source.parent / normalized).resolve()
        try:
            relative = resolved.relative_to(SRC_ROOT)
        except ValueError:
            return None
        return relative.parts[0] if relative.parts else None

    parts = normalized.split("/")
    if not parts:
        return None

    if parts[0] == "src" and len(parts) > 1:
        return parts[1]

    return parts[0]


def find_violations() -> list[str]:
    violations: list[str] = []

    for path in sorted(SRC_ROOT.rglob("*")):
        if path.suffix not in SOURCE_EXTENSIONS:
            continue

        owner = source_layer(path)
        forbidden = FORBIDDEN_INCLUDES.get(owner, set())
        if not forbidden:
            continue

        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
            match = INCLUDE_RE.match(line)
            if not match:
                continue

            dependency = include_layer(match.group(1), path)
            if dependency in forbidden:
                relative = path.relative_to(PROJECT_ROOT).as_posix()
                violations.append(
                    f"{relative}:{line_number}: {owner} must not include {dependency}: {match.group(1)}"
                )

    return violations


def main() -> int:
    violations = find_violations()
    if violations:
        print("Architecture boundary violations found:", file=sys.stderr)
        for violation in violations:
            print(f"  {violation}", file=sys.stderr)
        return 1

    print("Architecture boundary check passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
