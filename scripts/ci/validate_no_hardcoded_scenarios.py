#!/usr/bin/env python3
"""Lightweight guard against reintroducing hardcoded playable scenario data."""

from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]

DEFAULT_FACTORY = "makeSmallObjectiveRunArenaConfig("
FACTORY_ALLOWED_PREFIXES = (
    Path("src/battle_core"),
    Path("tests/unit/core"),
)
FACTORY_ALLOWED_FILES = {
    Path("src/battle_backend/ScenarioConfig.cpp"),
}


def repo_files() -> list[Path]:
    roots = [ROOT / "src", ROOT / "tests", ROOT / "tools"]
    files: list[Path] = []
    for root in roots:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.is_file() and path.suffix in {".cpp", ".hpp", ".py", ".mjs", ".ts", ".tsx", ".js"}:
                files.append(path)
    return files


def is_allowed_factory_use(relative: Path) -> bool:
    if relative in FACTORY_ALLOWED_FILES:
        return True
    return any(relative == prefix or prefix in relative.parents for prefix in FACTORY_ALLOWED_PREFIXES)


def main() -> int:
    errors: list[str] = []
    for path in repo_files():
        relative = path.relative_to(ROOT)
        text = path.read_text(encoding="utf-8", errors="replace")
        if DEFAULT_FACTORY in text and not is_allowed_factory_use(relative):
            errors.append(f"{relative}: default arena factory is forbidden outside pure core/unit-core tests")

    if errors:
        print("Hardcoded scenario validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1

    print("No hardcoded scenario blockers found.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
