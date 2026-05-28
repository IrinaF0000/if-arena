# Task 0050: Paired config-driven gameplay scenario tests

## Goal

Add shared config-driven gameplay scenario coverage with both desktop/TCP and mobile/WebSocket variants for every gameplay scenario config.

## Scope

Allowed files:

- `tests/scenarios/*.json`
- `tests/integration/gameplay/**`
- `tests/integration/desktop/**`
- `tests/integration/mobile/**`
- `scripts/ci/validate_gameplay_scenario_pairs.py`
- `tests/ci/validate_gameplay_scenario_pairs_tests.py`
- `docs/agent-runs/0050-paired-config-driven-gameplay-scenarios.md`
- `docs/agent-tasks/0050-paired-config-driven-gameplay-scenarios.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay rules
- protocol schemas
- Qt client source
- Telegram Mini App source
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Define shared scenario JSON for a full Objective Run capture route.
- Keep route commands, ticks, expected events, and expected assertions in scenario config, not test code.
- Add a generic scenario runner that loads scenario config and runs the same scenario through desktop/TCP and mobile/WebSocket transport paths.
- Add desktop and mobile wrapper tests for each gameplay scenario.
- Add paired scenario validator requiring both desktop and mobile coverage for each gameplay scenario config.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python tests/ci/validate_gameplay_scenario_pairs_tests.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, for local process/network test harness behavior
- Performance-Agent: no
- Frontend-Agent: yes, for mobile/WebSocket coverage path
- Qt-Agent: yes, for desktop/TCP coverage path
- CI/CD-Agent: yes, for new `scripts/ci/**` validator
- Verification-Agent: yes

## Acceptance criteria

- The same scenario JSON drives both desktop and mobile gameplay scenario tests.
- The full capture route reaches objective pickup and capture through real server commands.
- The paired validator fails when a desktop or mobile wrapper is missing and passes with both wrappers present.
- No gameplay route or map data is hardcoded in test code.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to remove paired gameplay scenario coverage.
