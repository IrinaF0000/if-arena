#!/usr/bin/env python3
"""Validate paired desktop and mobile coverage for gameplay scenario configs."""

from __future__ import annotations

import json
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCENARIO_DIR = ROOT / "tests" / "scenarios"
DESKTOP_DIR = ROOT / "tests" / "integration" / "desktop"
MOBILE_DIR = ROOT / "tests" / "integration" / "mobile"


def gameplay_scenarios() -> list[Path]:
    scenarios: list[Path] = []
    for path in sorted(SCENARIO_DIR.glob("*.json")):
        data = json.loads(path.read_text(encoding="utf-8"))
        if "gameScenario" in data:
            scenarios.append(path)
    return scenarios


def expected_wrapper(directory: Path, scenario: Path, suffix: str) -> Path:
    return directory / f"{scenario.stem}_{suffix}.py"


def validate_pairs() -> list[str]:
    errors: list[str] = []
    for scenario in gameplay_scenarios():
        desktop = expected_wrapper(DESKTOP_DIR, scenario, "desktop")
        mobile = expected_wrapper(MOBILE_DIR, scenario, "mobile")
        if not desktop.exists():
            errors.append(f"{scenario.relative_to(ROOT)} missing desktop wrapper {desktop.relative_to(ROOT)}")
        if not mobile.exists():
            errors.append(f"{scenario.relative_to(ROOT)} missing mobile wrapper {mobile.relative_to(ROOT)}")
    return errors


def main() -> int:
    errors = validate_pairs()
    if errors:
        print("Gameplay scenario pair validation failed:")
        for error in errors:
            print(f"- {error}")
        return 1
    print("Gameplay scenario desktop/mobile pairs OK.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
