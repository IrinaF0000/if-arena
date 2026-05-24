#!/usr/bin/env python3
from __future__ import annotations

import importlib.util
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / "scripts" / "ci" / "validate_architecture_boundaries.py"

spec = importlib.util.spec_from_file_location("validate_architecture_boundaries", SCRIPT)
module = importlib.util.module_from_spec(spec)
assert spec.loader is not None
spec.loader.exec_module(module)


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def validate_fixture(files: dict[str, str]) -> list[str]:
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for rel, text in files.items():
            write(root / rel, text)
        return module.validate(root)


def test_minimal_valid_repo_passes() -> None:
    errors = validate_fixture(
        {
            "CMakeLists.txt": """
add_library(battle_core STATIC src/battle_core/BattleEngine.cpp)
add_library(battle_protocol STATIC src/battle_protocol/Protocol.cpp)
add_executable(battle_qt_client src/battle_qt_client/main.cpp)
target_link_libraries(battle_qt_client PRIVATE battle_protocol Qt6::Widgets)
""",
            "src/battle_core/BattleEngine.cpp": '#include "BattleEngine.hpp"\n#include <vector>\n',
            "src/battle_core/BattleEngine.hpp": "#include <cstdint>\n",
            "src/battle_qt_client/main.cpp": '#include "ProtocolJson.hpp"\n',
            "assets/player.svg": '<svg xmlns="http://www.w3.org/2000/svg"><circle r="1"/></svg>',
        }
    )
    require(errors == [], f"expected no errors, got {errors}")


def test_core_forbidden_includes_fail() -> None:
    errors = validate_fixture(
        {
            "src/battle_core/BattleEngine.cpp": '#include <iostream>\n#include "battle_protocol/Protocol.hpp"\n',
        }
    )
    text = "\n".join(errors)
    require("forbidden IO include" in text, "iostream include should fail")
    require("forbidden battle_core include" in text, "protocol include should fail")


def test_cmake_forbidden_dependency_fails() -> None:
    errors = validate_fixture(
        {
            "CMakeLists.txt": """
add_library(battle_core STATIC src/battle_core/BattleEngine.cpp)
target_link_libraries(battle_core PUBLIC battle_protocol)
""",
        }
    )
    require(any("battle_core links forbidden dependency battle_protocol" in error for error in errors), "core link should fail")


def test_svg_safety_and_external_skip() -> None:
    errors = validate_fixture(
        {
            "assets/bad.svg": '<svg><script>alert(1)</script><image href="https://example.test/a.png"/></svg>',
            "external/reference/bad.svg": '<svg><script>alert(1)</script></svg>',
        }
    )
    text = "\n".join(errors)
    require("assets/bad.svg" in text, "production SVG should be checked")
    require("external/reference/bad.svg" not in text, "external snapshot SVG should be skipped")


def main() -> int:
    tests = [
        test_minimal_valid_repo_passes,
        test_core_forbidden_includes_fail,
        test_cmake_forbidden_dependency_fails,
        test_svg_safety_and_external_skip,
    ]
    for test in tests:
        test()
    print("validate_architecture_boundaries tests OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
