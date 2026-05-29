#!/usr/bin/env python3

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tests" / "integration" / "gameplay"))

from scenario_runner import run_scenario_file


if __name__ == "__main__":
    run_scenario_file(Path("tests/scenarios/rematch_same_screen.json"), "desktop")
    print("[PASS] rematch_same_screen_desktop")
