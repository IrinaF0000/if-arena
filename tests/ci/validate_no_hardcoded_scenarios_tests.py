#!/usr/bin/env python3
"""Self-test for the no-hardcoded-scenarios validator."""

from __future__ import annotations

import importlib.util
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "ci" / "validate_no_hardcoded_scenarios.py"


def load_module():
    spec = importlib.util.spec_from_file_location("validate_no_hardcoded_scenarios", SCRIPT)
    if spec is None or spec.loader is None:
        raise AssertionError("unable to load validator module")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    module = load_module()
    assert module.is_allowed_factory_use(Path("src/battle_core/ArenaConfig.hpp"))
    assert module.is_allowed_factory_use(Path("tests/unit/core/BattleEngineFacadeTests.cpp"))
    assert module.is_allowed_factory_use(Path("src/battle_backend/ScenarioConfig.cpp"))
    assert not module.is_allowed_factory_use(Path("src/battle_backend/BackendFoundation.cpp"))
    assert not module.is_allowed_factory_use(Path("tests/integration/server/tcp_vertical_slice_smoke.py"))
    print("[PASS] validate_no_hardcoded_scenarios_tests")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
