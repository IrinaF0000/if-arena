# Task 0076: Corrective crow patrol radius

## Goal

Restore deterministic crow movement by separating crow patrol radius from hazard effect range.

## Scope

Allowed files:

- `src/battle_core/**`
- `src/battle_backend/ScenarioConfig.*`
- `config/scenarios/arena_small_objective_run.json`
- `tests/unit/core/**`
- `tests/unit/backend/**`
- `docs/agent-runs/0076-corrective-crow-patrol-radius.md`
- `docs/agent-tasks/0076-corrective-crow-patrol-radius.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- client authority changes
- transport/protocol envelope changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Crow movement uses `patrolRadius`, not hazard effect `range`.
- Hazard `range` and `radius` still define gameplay effect reach and snapshot metadata.
- Scenario config may specify `patrolRadius` for crow hazards.
- Invalid crow patrol radius is rejected by validation.
- Existing deterministic hazard behavior remains stable.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- C++ quality rules
- Testing rules

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure -R "battle_core|battle_backend"`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- A crow with a short effect range and larger patrol radius changes patrol cells deterministically.
- Crow patrol candidates remain within `patrolRadius`.
- Effect range remains unchanged in snapshots and hit logic.
- Scenario parser/validation rejects an impossible crow patrol radius.

## Rollback note

Revert this task commit to return crow patrol movement to the previous `range`-based behavior.
