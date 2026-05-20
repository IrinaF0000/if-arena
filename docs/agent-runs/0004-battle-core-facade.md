# 0004 Battle Core Facade

Date: 2026-05-19

## Changed files

- `src/battle_core/BattleEngine.hpp` - added minimal server-facing facade DTOs and `BattleEngine`.
- `src/battle_core/placeholder.cpp` - implemented the minimal deterministic facade.
- `tests/unit/core/BattleEngineFacadeTests.cpp` - added deterministic facade tests.
- `src/battle_core/README.md` - documented the public facade.
- `docs/architecture/TECHNICAL_ARCHITECTURE.md` - documented initial facade role.
- `docs/project/CORE_EXTRACTION_CHECKLIST.md` - marked completed facade extraction items.

## Checks run

- `cmake -S . -B cmake-build-0004 -G Ninja -DBATTLE_BUILD_QT_CLIENT=OFF -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build cmake-build-0004 --parallel` - passed.
- Manual unit compile: `g++ -std=c++20 -Wall -Wextra -Wpedantic -I src/battle_core src/battle_core/placeholder.cpp tests/unit/core/BattleEngineFacadeTests.cpp -o cmake-build-0004/BattleEngineFacadeTests.exe` - passed.
- `cmake-build-0004/BattleEngineFacadeTests.exe` - passed.
- `ctest --test-dir cmake-build-0004 --output-on-failure` - passed; no tests are registered in root CMake yet.
- `python scripts/ci/scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed with Python shim in Git Bash.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed.
- C++ dependency text check over `src/battle_core/*.cpp` and `*.hpp` - no transport/UI/server dependency terms found.

## Risks and follow-up

- Root CMake does not yet register `tests/unit` executables, so the new facade test is compiled and run manually.
- Attack, interact, objectives, hazards, cooldowns, and full old reference scenario behavior remain for later extraction packets.
- Architecture-Agent and Verification-Agent review are required before treating the facade as stable.
