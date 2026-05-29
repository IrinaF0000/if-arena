# Task 0061: Objective reset staging

## Goal

Make objective reset after capture visually readable by keeping the objective at the capture point during the configured respawn phase, then returning it to spawn on the authoritative respawn event.

## Scope

Allowed files:

- `src/battle_core/**`
- `tests/unit/core/**`
- `tests/integration/**`
- `tests/scenarios/**`
- `docs/agent-runs/0061-objective-reset-staging.md`
- `docs/agent-tasks/0061-objective-reset-staging.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- frontend/Qt presentation beyond what authoritative snapshots already expose
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Capture increments score exactly once and emits capture/score events.
- Non-terminal capture enters the configured respawn phase with objective position still at the capture point.
- Objective returns to configured spawn only when respawn delay completes.
- Terminal capture keeps objective at the capture point.

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
- `python tests/integration/desktop/rematch_same_screen_desktop.py`
- `python tests/integration/mobile/rematch_same_screen_mobile.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Unit tests prove objective stays at capture point during respawn staging.
- Paired desktop/mobile scenario tests prove objective is not at spawn immediately after capture and returns after delay.
- Respawn timing remains driven by config.

## Rollback note

Revert this task commit to restore immediate objective position reset on capture.
