#!/usr/bin/env python3
"""Mobile/WebSocket coverage for the config-driven full capture scenario."""

from __future__ import annotations

from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tests" / "integration" / "gameplay"))

from scenario_runner import run_scenario_file  # noqa: E402


def main() -> int:
    run_scenario_file(ROOT / "tests/scenarios/objective_run_full_capture.json", "mobile")
    print("[PASS] objective_run_full_capture_mobile")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
