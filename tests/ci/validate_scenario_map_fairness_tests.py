#!/usr/bin/env python3
"""Self-tests for scenario map fairness validation."""

from __future__ import annotations

import importlib.util
import json
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "ci" / "validate_scenario_map_fairness.py"

spec = importlib.util.spec_from_file_location("validate_scenario_map_fairness", SCRIPT)
assert spec is not None and spec.loader is not None
module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = module
spec.loader.exec_module(module)


def base_scenario() -> dict:
    return {
        "scenario": "test",
        "mode": "objective_run",
        "map": {
            "width": 21,
            "height": 13,
            "symmetry": "rotational_180",
            "center": {"x": 10, "y": 6},
            "blueBase": {"x": 10, "y": 11, "radius": 1.5},
            "redBase": {"x": 10, "y": 1, "radius": 1.5},
            "objectiveSpawn": {"x": 10, "y": 6},
            "obstacles": [
                {"x": 6, "y": 4},
                {"x": 14, "y": 8},
            ],
        },
        "players": {
            "blueSpawn": {"x": 10, "y": 10},
            "redSpawn": {"x": 10, "y": 2},
        },
        "hazards": [
            {"id": "left", "kind": "mine", "x": 7, "y": 5, "radius": 0.7, "range": 1.0, "damage": 12, "effect": "damage_drop_objective", "trigger": "proximity", "icon": "hazard_mine", "cooldownTicks": 30},
            {"id": "right", "kind": "mine", "x": 13, "y": 7, "radius": 0.7, "range": 1.0, "damage": 12, "effect": "damage_drop_objective", "trigger": "proximity", "icon": "hazard_mine", "cooldownTicks": 30},
            {"id": "center", "kind": "crow", "x": 10, "y": 6, "radius": 0.65, "range": 1.5, "damage": 6, "effect": "damage_drop_objective", "trigger": "proximity", "icon": "hazard_crow", "cooldownTicks": 8, "seed": 3},
        ],
    }


def validate_temp(scenario: dict) -> tuple[str, ...]:
    with tempfile.TemporaryDirectory() as temp_dir:
        path = Path(temp_dir) / "scenario.json"
        path.write_text(json.dumps(scenario), encoding="utf-8")
        return module.validate_scenario(path).errors


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def valid_repo_scenario_passes() -> None:
    result = module.validate_scenario(ROOT / "config" / "scenarios" / "arena_small_objective_run.json")
    require(not result.errors, f"repo scenario should pass: {result.errors}")


def asymmetric_obstacle_is_rejected() -> None:
    scenario = base_scenario()
    scenario["map"]["obstacles"].append({"x": 3, "y": 4})
    errors = validate_temp(scenario)
    require(any("obstacle missing rotational pair" in error for error in errors), "asymmetric obstacle is rejected")


def unmatched_hazard_is_rejected() -> None:
    scenario = base_scenario()
    scenario["hazards"].pop(0)
    errors = validate_temp(scenario)
    require(any("hazard missing equivalent rotational pair" in error for error in errors), "unmatched hazard is rejected")


def one_cell_choke_is_rejected() -> None:
    scenario = base_scenario()
    scenario["map"]["obstacles"] = []
    for x in range(21):
        if x != 10:
            scenario["map"]["obstacles"].append({"x": x, "y": 6})
    errors = validate_temp(scenario)
    require(any("required one-cell choke" in error for error in errors), "one-cell choke is rejected")


def main() -> int:
    tests = [
        valid_repo_scenario_passes,
        asymmetric_obstacle_is_rejected,
        unmatched_hazard_is_rejected,
        one_cell_choke_is_rejected,
    ]
    for test in tests:
        test()
    print("validate_scenario_map_fairness self-tests OK.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
