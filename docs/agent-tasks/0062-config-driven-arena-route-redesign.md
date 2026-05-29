# Task 0062: Config-driven arena route redesign

## Goal

Redesign the default arena through scenario config so the objective route is less direct while preserving desktop/mobile consistency and config-driven tests.

## Scope

Allowed files:

- `config/scenarios/arena_small_objective_run.json`
- `scripts/ci/validate_scenario_map_fairness.py`
- `tests/ci/validate_scenario_map_fairness_tests.py`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/integration/**`
- `tests/scenarios/**`
- `docs/agent-runs/0062-config-driven-arena-route-redesign.md`
- `docs/agent-tasks/0062-config-driven-arena-route-redesign.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay rules in `battle_core`
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Default scenario direct center-lane route is blocked by config-authored obstacles.
- Rotational symmetry and pathability are preserved.
- Scenario tests navigate using config roles instead of hardcoded direct movement paths.
- Map fairness validator rejects routes below the configured minimum length.

## Required quality gates

- Gate A. Every implementation task
- Gate C. Backend/gameplay authority
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity

## Required checks

- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `python tests/integration/desktop/rematch_same_screen_desktop.py`
- `python tests/integration/mobile/rematch_same_screen_mobile.py`
- `python scripts/ci/validate_scenario_map_fairness.py`
- `python tests/ci/validate_scenario_map_fairness_tests.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- The default scenario remains `arena_small_objective_run`.
- The shortest route to/from objective is longer than the previous direct lane.
- Desktop and mobile scenario specs continue passing against the same config.
- No client or test hardcodes a map-specific route.

## Rollback note

Revert this task commit to restore the previous direct-lane scenario and scenario test paths.
