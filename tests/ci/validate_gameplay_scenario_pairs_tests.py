#!/usr/bin/env python3
"""Self-test for gameplay scenario pair validation naming rules."""

from __future__ import annotations

import importlib.util
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "ci" / "validate_gameplay_scenario_pairs.py"


def load_module():
    spec = importlib.util.spec_from_file_location("validate_gameplay_scenario_pairs", SCRIPT)
    if spec is None or spec.loader is None:
        raise AssertionError("unable to load pair validator")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    module = load_module()
    scenario = Path("tests/scenarios/objective_run_full_capture.json")
    assert module.expected_wrapper(Path("tests/integration/desktop"), scenario, "desktop") == Path(
        "tests/integration/desktop/objective_run_full_capture_desktop.py"
    )
    assert module.expected_wrapper(Path("tests/integration/mobile"), scenario, "mobile") == Path(
        "tests/integration/mobile/objective_run_full_capture_mobile.py"
    )
    errors = module.validate_pairs()
    assert errors == [], errors
    print("[PASS] validate_gameplay_scenario_pairs_tests")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
