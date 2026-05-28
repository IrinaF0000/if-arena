# Task 0048: Scenario config single source of truth

## State

committed

## Coordinator Preflight

- Branch: `agent/0047-playable-stabilization`.
- Baseline: task 0047 commit `2906858`.
- Working tree at preflight had one untracked local file not owned by this task: `post_0046_playable_stabilization_agent_plan.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0048-scenario-config-single-source.md`.
- Allowed scope confirmed: scenario config examples/docs, backend scenario loader/injection, server app startup path, focused backend tests, new no-hardcoded-scenarios CI validator, focused task/run docs.
- Forbidden scope confirmed: no workflows, no deploy files, no `battle_core` production code, no client source, no generated frontend `dist/**`, no release tag, no push, no merge into `master` or `main` before final post-0046 acceptance.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Architecture/Security/CI-CD Review-Agent -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Scenario Source Audit

Initial classification:

- Allowed config data: `config/scenarios/arena_small_objective_run.json`.
- Forbidden normal-startup hardcoded scenario data: `src/battle_backend/BackendFoundation.cpp` calls `battle_core::makeSmallObjectiveRunArenaConfig()` in `MatchManager::makeMatchConfig`.
- Generic pure-core fallback/unit-test data: `src/battle_core/ArenaConfig.hpp` default arena helper and core unit tests using it.
- Generic protocol/client scenario id references: create-match payloads that request `arena_small_objective_run`.
- Test harness payload references: smoke/security/load tests hardcode only the scenario id today; this remains allowed until full scenario runner work in later tasks.

## Implementation-Agent Prompt

Implement only task 0048 from Priority 2 of `post_0046_playable_stabilization_agent_plan.md`. Add a scenario JSON loading/injection path outside `battle_core`, make normal server startup use `config/scenarios/arena_small_objective_run.json` by default or a configured path, add validation and focused backend tests, and add a lightweight CI validator for obvious hardcoded scenario data. Do not change gameplay rules, map redesign, client code, workflows, deploy files, generated outputs, or merge to `master`/`main`.

## Implementation-Agent

Completed.

Changed files:

- `.gitignore`
- `CMakeLists.txt`
- `config/examples/README.md`
- `config/examples/server.local.json`
- `config/examples/server.public.json`
- `config/scenarios/README.md`
- `config/scenarios/arena_small_objective_run.json`
- `config/test_scenarios/invalid_asymmetric_obstacle.json`
- `docs/agent-tasks/0048-scenario-config-single-source.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0048-scenario-config-single-source.md`
- `scripts/ci/validate_no_hardcoded_scenarios.py`
- `src/battle_backend/ScenarioConfig.cpp`
- `src/battle_backend/ScenarioConfig.hpp`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/MatchLoop.hpp`
- `src/battle_server_app/main.cpp`
- `tests/ci/validate_no_hardcoded_scenarios_tests.py`
- `tests/unit/backend/SessionRegistryTests.cpp`

Implementation summary:

- Added a backend scenario parser for the existing JSON scenario format. It validates ids, mode, dimensions, bases, spawns, objective, obstacles, hazards, combat fields, timing, and then delegates arena invariants to `battle_core::validateArenaConfig`.
- Injected `PlayableScenarioConfig` into `MatchManager` so normal server startup can provide scenario data without `battle_core` reading files or JSON.
- Made `battle_server_app` load `game.scenarioPath`, defaulting to `config/scenarios/arena_small_objective_run.json`, before listeners start.
- Updated the default scenario JSON to match the current playable arena data, including the crow hazard and combat/player metadata that had lived only in C++ defaults.
- Kept `makeSmallObjectiveRunArenaConfig()` as a pure fallback/unit-test helper through `makeFallbackObjectiveRunScenario()`, not as the normal server startup path.
- Added a focused no-hardcoded-scenarios CI validator and a self-test for its allow/deny rules.
- Added backend tests that load valid and invalid scenario JSON files from config paths instead of embedding scenario layout data in test code.
- Added `.gitignore` coverage for nested Python cache output produced by the new validator self-test.

## Test Impact Matrix

Changed behavior:

- Normal server app startup validates and loads the playable scenario from JSON before constructing the backend match manager.
- Backend matches can now be constructed from injected scenario config instead of always calling the core default arena helper.
- Scenario JSON now owns the current playable arena obstacles, hazards, player speed/HP, objective, combat, and timing metadata.

Tests added/updated:

- Happy path: backend unit test loads `config/scenarios/arena_small_objective_run.json` and verifies scenario id, obstacle count, hazard count, and tick conversion.
- Corner cases: backend unit test loads `config/test_scenarios/invalid_asymmetric_obstacle.json` and verifies arena symmetry rejection.
- Invalid input / hostile input: scenario parser rejects unsupported ids/modes, missing required objects/fields, invalid numbers, unsupported hazards, invalid timing, and invalid arena validation results.
- Authority / ownership: `battle_core` remains file/JSON-free; clients still request a scenario id only and do not own scenario state.
- Resource bounds / performance: no runtime queue, tick, broadcast, or backpressure bounds changed.
- Regression: full CTest suite, WebSocket smoke, server `--check-config`, warning-clean build, structure, secret, architecture, agent, playable-sequence, no-hardcoded-scenarios, and diff checks.
- Manual UI checks: none; no UI rendering/input behavior changed.

Not tested and why:

- Linux runtime execution was not available in this Windows workspace. The server executable path and scenario loading were exercised locally with Windows builds, and the parser/validator are platform-neutral.

## Verification-Agent

Passed.

Checks run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-werror -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
- `cmake --build build-werror --parallel`
- `python tests/frontend/websocket_local_smoke.py`
- `build\battle_server_app.exe --config config\examples\server.local.json --check-config`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python tests/ci/validate_no_hardcoded_scenarios_tests.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash -lc 'export PATH="/usr/bin:$PATH"; function python3(){ python "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'`
- `git status --ignored --short`
- `git diff --check`

Notes:

- `git diff --check` reported only line-ending conversion warnings and no whitespace errors.
- `scripts/ci/__pycache__/`, build directories, and generated local configs are ignored and not staged.

## Review-Agent

Architecture-Agent: approved. Scenario file IO and JSON parsing stay outside `battle_core`; `battle_core` receives value-type config only. Normal server startup now injects scenario config through backend/server app boundaries.

Security-Agent: approved. Scenario config is treated as untrusted startup input, validated before listeners start, and errors are structured without secrets. Secret scan passed.

CI/CD-Agent: approved. The new CI validator is scoped, advisory-by-command in this task, has a self-test, and does not modify workflows.

Performance Review Agent: not required. No tick loop, queues, workers, or load limits changed.

## Commit-Agent

Completed.

- Commit: current task commit, `backend: load playable scenario config`
- Branch: `agent/0047-playable-stabilization`
- No merge to `master` or `main`.
- No push or release tag.

## Coordinator Closeout

Task 0048 is terminal on the stabilization branch. The untracked local plan file `post_0046_playable_stabilization_agent_plan.md` remains unstaged.
