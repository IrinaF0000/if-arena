#!/usr/bin/env python3
from __future__ import annotations

import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tests" / "integration" / "gameplay"))

from scenario_runner import run_scenario_file  # noqa: E402


def main() -> int:
    run_scenario_file(ROOT / "tests" / "scenarios" / "objective_event_sequence.json", "desktop")
    print("[PASS] objective_event_sequence_desktop")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
