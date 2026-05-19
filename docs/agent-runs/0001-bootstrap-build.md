# 0001 Bootstrap Build Run

Date: 2026-05-19

## Changed files

- `docs/project/REPOSITORY_BOOTSTRAP.md` - aligned bootstrap module names with `PROJECT_MAP.md`.
- `docs/agent-runs/0001-bootstrap-build.md` - recorded this run.

## Checks run

- `cmake -S . -B build -DBATTLE_BUILD_QT_CLIENT=OFF` - blocked: default Visual Studio generator could not find a C++ compiler.
- `cmake -S . -B build-ninja -G Ninja -DBATTLE_BUILD_QT_CLIENT=OFF` - passed.
- `cmake --build build-ninja --parallel` - passed.
- `ctest --test-dir build-ninja --output-on-failure` - passed; no tests were found.
- Frontend `package.json` script check - `typecheck` and `lint` are defined.
- `npm --version` - blocked: `npm` is not on PATH in this environment.
- `python.exe scripts/agent/validate_agent_harness.py` - passed.
- `bash scripts/ci/validate_structure.sh` via Git Bash with bundled Python shim - failed because secret scan reports existing findings under `external/battle_simulation_snapshot/`.
- `python.exe scripts/ci/scan_secrets.py` - failed with the same existing snapshot findings.
- `git diff --check` - passed.

## Risks and blockers

- Default CMake configure is blocked by local Visual Studio compiler discovery, but the placeholder build succeeds with the available MinGW/Ninja toolchain.
- Frontend npm commands could not be executed because `npm` is unavailable; required scripts are present in `frontend/telegram_mini_app/package.json`.
- Secret scanning currently reports findings inside `external/battle_simulation_snapshot/`, which this task forbids modifying.
