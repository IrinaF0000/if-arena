# Task 0053: Objective event visibility

## Goal

Make objective pickup, drop, repickup, capture, carrier state, and score changes visible and tested in both desktop/TCP and mobile/WebSocket paths.

## Scope

Allowed files:

- `tests/scenarios/*.json`
- `tests/integration/gameplay/**`
- `tests/integration/desktop/**`
- `tests/integration/mobile/**`
- `frontend/telegram_mini_app/src/**`
- `src/battle_qt_client/**`
- `docs/agent-runs/0053-objective-event-visibility.md`
- `docs/agent-tasks/0053-objective-event-visibility.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Verify objective pickup, drop, capture, carrier, and score events are exposed by the existing protocol.
- Add player-facing objective event text in both clients.
- Keep clients presentation-only; clients must not infer or mutate authoritative objective state.
- Extend config-driven paired scenarios to cover pickup, drop, repickup, capture, and score increment without hardcoding event sequences in test code.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_scenario_map_fairness.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- Qt MinGW configure/build/CTest when Qt is available
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, for frontend/Qt event validation boundaries
- Performance-Agent: no
- Frontend-Agent: yes
- Qt-Agent: yes
- CI/CD-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Objective event sequence is tested through both desktop/TCP and mobile/WebSocket wrappers.
- Event expectations live in scenario config, not test code.
- Web and Qt clients show readable objective messages for local/enemy perspective.
- Existing full capture scenario still passes.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to remove the objective event sequence scenario and client event-message polish.
