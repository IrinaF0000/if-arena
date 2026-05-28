# Task 0050: Paired config-driven gameplay scenario tests

## State

committed

## Coordinator Preflight

- Branch: `agent/0047-playable-stabilization`.
- Baseline: task 0049 commit `fb28acd`.
- Working tree at preflight had one untracked local file not owned by this task: `post_0046_playable_stabilization_agent_plan.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0050-paired-config-driven-gameplay-scenarios.md`.
- Allowed scope confirmed: scenario JSON, shared gameplay runner, desktop/mobile wrappers, paired validator, focused task/run docs.
- Forbidden scope confirmed: no workflows, no deploy files, no source gameplay rules, no protocol schemas, no client source, no generated frontend `dist/**`, no release tag, no push, no merge into `master` or `main` before final post-0046 acceptance.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Architecture/Security/Frontend/Qt/CI-CD Review-Agent -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0050 from Priority 4 of `post_0046_playable_stabilization_agent_plan.md`. Add shared config-driven gameplay scenario JSON and run it through paired desktop/TCP and mobile/WebSocket local server tests. Add a validator that proves every gameplay scenario has both wrappers. Do not change gameplay rules, protocol schemas, client source, workflows, deploy files, generated outputs, or merge to `master`/`main`.

## Implementation-Agent

Completed.

Changed files:

- `tests/scenarios/objective_run_full_capture.json`
- `tests/integration/gameplay/README.md`
- `tests/integration/gameplay/scenario_runner.py`
- `tests/integration/desktop/objective_run_full_capture_desktop.py`
- `tests/integration/mobile/objective_run_full_capture_mobile.py`
- `scripts/ci/validate_gameplay_scenario_pairs.py`
- `tests/ci/validate_gameplay_scenario_pairs_tests.py`
- `docs/agent-tasks/0050-paired-config-driven-gameplay-scenarios.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0050-paired-config-driven-gameplay-scenarios.md`

Implementation summary:

- Added `objective_run_full_capture.json` with the full capture route, tick timing, movement assertions, and expected objective/score assertions in config.
- Added a shared local scenario runner that starts `battle_server_app`, connects two clients, and runs the same scenario over desktop/TCP or mobile/WebSocket transport paths.
- Added paired desktop and mobile wrapper tests for the full capture scenario.
- Added `validate_gameplay_scenario_pairs.py` plus a self-test to require desktop and mobile wrappers for each gameplay scenario JSON.
- Kept `movement_reliability.json` outside paired gameplay validation because it is a Qt input-model scenario and does not declare `gameScenario`.

## Test Impact Matrix

Changed behavior:

- Test coverage now includes full Objective Run pickup/capture through real server commands on both raw TCP and WebSocket paths.
- CI-style validation can prove gameplay scenario configs have paired desktop and mobile wrappers.

Tests added/updated:

- Happy path: `objective_run_full_capture` reaches pickup, return, capture, and blue score increment over TCP and WebSocket.
- Corner cases: runner verifies movement snapshots change smoothly and rejects unexpected large player-position jumps.
- Invalid input / hostile input: no parser, auth, or hostile network-input behavior changed in this task; existing negative suites remain in place.
- Authority / ownership: clients send only configured intentions; server snapshots/events remain authoritative for pickup/capture/score assertions.
- Resource bounds / performance: local server config uses bounded frame/message sizes, command queues, outbound queues, rate limits, and finite `--max-clients`.
- Regression: full CTest suite, paired scenario wrappers, no-hardcoded-scenarios, pair validator/self-test, architecture, secret, agent, playable-sequence, and diff checks.
- Manual UI checks: none; this task adds automated transport-level gameplay coverage, not live UI rendering.

Not tested and why:

- Real Qt window and real browser WebView interactions are not driven here. The wrappers exercise desktop/TCP and mobile/WebSocket protocol paths; later visual tasks can add UI-level capture checks.

## Verification-Agent

Passed.

Checks run:

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

Notes:

- `git diff --check` reported only line-ending conversion warnings and no whitespace errors.
- Generated `__pycache__/` directories are ignored and not staged.

## Review-Agent

Architecture-Agent: approved. Scenario tests exercise existing transport/backend boundaries without moving authority into clients or changing gameplay/protocol code.

Security-Agent: approved. The local test harness uses bounded local-only server configs, demo auth only, finite max clients, and no secrets.

Frontend-Agent: approved. Mobile coverage uses the WebSocket client protocol path and validates server-authored snapshots without adding frontend authority.

Qt-Agent: approved. Desktop coverage uses the raw TCP path that the Qt client uses; no Qt source is changed in this task.

CI/CD-Agent: approved. The new validator is scoped, has a self-test, and does not modify workflow files.

Performance Review Agent: not required. No runtime performance path changed.

## Commit-Agent

Completed.

- Commit: current task commit, `test: add paired gameplay scenarios`
- Branch: `agent/0047-playable-stabilization`
- No merge to `master` or `main`.
- No push or release tag.

## Coordinator Closeout

Task 0050 is terminal on the stabilization branch. The untracked local plan file `post_0046_playable_stabilization_agent_plan.md` remains unstaged.
