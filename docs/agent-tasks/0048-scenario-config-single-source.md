# Task 0048: Scenario config single source of truth

## Goal

Make the default playable Objective Run scenario loaded from scenario JSON for normal server startup, while keeping `battle_core` isolated from filesystem and JSON parsing.

## Scope

Allowed files:

- `CMakeLists.txt`
- `.gitignore`
- `config/examples/*.json`
- `config/examples/README.md`
- `config/scenarios/*.json`
- `config/scenarios/README.md`
- `config/test_scenarios/*.json`
- `scripts/ci/validate_no_hardcoded_scenarios.py`
- `src/battle_backend/**`
- `src/battle_server_app/main.cpp`
- `tests/unit/backend/**`
- `tests/ci/**`
- `docs/agent-runs/0048-scenario-config-single-source.md`
- `docs/agent-tasks/0048-scenario-config-single-source.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- `battle_core` production code
- Qt client source
- Telegram Mini App source
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Classify current scenario data sources in the run note.
- Add a config-driven scenario loading path outside `battle_core`.
- Keep `battle_core` accepting value-type scenario/config data only; it must not read files or JSON.
- Ensure normal server startup loads `config/scenarios/arena_small_objective_run.json` or a configured scenario path.
- Keep `makeSmallObjectiveRunArenaConfig()` only as a pure-core fallback/unit-test helper, not as normal server startup data.
- Add validation for loaded scenario configs: dimensions, required points, bounds, duplicate/blocked cells, objective, bases, spawns, obstacles, hazards, and rotational symmetry.
- Add a CI validator that catches obvious hardcoded scenario data and forbidden default arena factory use in integration/server paths.
- Ensure Python cache output from the new validator tests remains ignored.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, for config parsing and startup behavior
- Performance-Agent: no
- Frontend-Agent: no
- Qt-Agent: no
- CI/CD-Agent: yes, for new `scripts/ci/**` validator
- Verification-Agent: yes

## Acceptance criteria

- Normal server startup no longer constructs the playable arena from hardcoded C++ scenario coordinates.
- The scenario JSON is validated before match startup.
- Unit tests cover valid scenario load and invalid scenario rejection.
- The no-hardcoded-scenarios validator passes and is documented in the run note.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to return to the pre-0048 default arena construction path.
