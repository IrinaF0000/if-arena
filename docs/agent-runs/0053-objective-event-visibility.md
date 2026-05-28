# Run 0053: Objective event visibility

## Status

State: committed
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0053-objective-event-visibility.md`

## Scope

- Extend config-driven scenario tests for objective pickup, drop, repickup, capture, and score.
- Improve objective event text in Telegram Mini App and Qt client.
- Do not modify workflows, deploy files, protected CI docs, tags, push, or merge into `master`/`main`.

## Progress

- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Test Impact Matrix

Changed behavior:
- Config-driven gameplay scenarios now verify objective pickup, drop, repickup, capture, and score-change event sequences through both TCP and WebSocket paths.
- Telegram Mini App and Qt client objective event text now uses local/enemy player perspective for pickup, drop, and capture.
- Telegram canvas feedback labels now include local/enemy objective action text.

Tests added/updated:
- Happy path: new paired `objective_event_sequence` desktop/mobile scenario covers pickup, attack-caused drop, lock-delayed repickup, capture, and blue score increment.
- Corner cases: scenario runner now supports event assertions from config and smooth movement for whichever actor moved, not only Blue.
- Invalid input / hostile input: existing protocol/frontend validation checks still pass; no new inbound client authority fields added.
- Authority / ownership: event text and canvas feedback are presentation-only; clients still render snapshots/events and send intentions only.
- Resource bounds / performance: event tracking is bounded by short local scenario execution; no server queue or runtime limit behavior changed.
- Regression: existing full-capture desktop/mobile scenarios, paired scenario validator, CTest, Qt CTest, frontend typecheck/lint/build, architecture, no-hardcoded, map fairness, and secret checks passed.
- Manual UI checks: not run interactively; automated Qt build verifies client code compiles and paired protocol scenarios verify event delivery.

Not tested and why:
- Full two-client visual manual play was not run in this non-interactive session.

## Checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `python tests/integration/desktop/objective_event_sequence_desktop.py` - passed.
- `python tests/integration/mobile/objective_event_sequence_mobile.py` - passed.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/mobile/objective_run_full_capture_mobile.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `python scripts/ci/validate_no_hardcoded_scenarios.py` - passed.
- `python scripts/ci/validate_scenario_map_fairness.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with approved escalation after sandbox-only esbuild path access failed.
- Qt MinGW configure with `BATTLE_BUILD_QT_CLIENT=ON` - passed.
- Qt MinGW build - passed.
- `ctest --test-dir build-qt-mingw --output-on-failure` - passed, 8/8 tests.
- `git diff --check` - passed with CRLF normalization warnings only.

## Reviews

- Architecture-Agent: approved; changes stay in scenario tests and presentation clients, with no game authority moved out of server/core.
- Security-Agent: approved; event rendering consumes validated server messages and does not trust client-owned state.
- Performance-Agent: not required.
- Frontend-Agent: approved; strict typecheck/lint/build passed and event text remains presentation-only.
- Qt-Agent: approved; Qt network event text compiles in the Qt target and Qt CTest passes.
- Verification-Agent: approved; required automated checks passed.

## Risks

- No merge to `master` or `main`, push, or tag was performed.
