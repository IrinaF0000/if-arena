# Task 0049: Desktop movement reliability

## State

committed

## Coordinator Preflight

- Branch: `agent/0047-playable-stabilization`.
- Baseline: task 0048 commit `17352bf`.
- Working tree at preflight had one untracked local file not owned by this task: `post_0046_playable_stabilization_agent_plan.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0049-desktop-movement-reliability.md`.
- Allowed scope confirmed: Qt client input model/UI wiring, Qt client tests, one movement scenario config, focused docs/task/run updates.
- Forbidden scope confirmed: no workflows, no `scripts/ci/**`, no deploy files, no gameplay authority/rules, no backend command validation changes, no Telegram source, no generated frontend `dist/**`, no release tag, no push, no merge into `master` or `main` before final post-0046 acceptance.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Architecture/Qt Review-Agent -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0049 from Priority 3 of `post_0046_playable_stabilization_agent_plan.md`. Add a config-driven movement reliability test and a bounded Qt desktop movement resend/retry model. Wire MainWindow to resend held movement at a controlled interval, retry after rejection, reset on match/disconnect, and show rejection feedback. Do not change gameplay authority, backend rules, scenario map design, CI workflows, deploy files, generated outputs, or merge to `master`/`main`.

## Implementation-Agent

Completed.

Changed files:

- `CMakeLists.txt`
- `src/battle_qt_client/game/MovementInputController.cpp`
- `src/battle_qt_client/game/MovementInputController.hpp`
- `src/battle_qt_client/tests/MovementInputControllerTests.cpp`
- `src/battle_qt_client/ui/MainWindow.cpp`
- `src/battle_qt_client/ui/MainWindow.hpp`
- `src/battle_qt_client/ui/README.md`
- `tests/scenarios/movement_reliability.json`
- `docs/agent-tasks/0049-desktop-movement-reliability.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0049-desktop-movement-reliability.md`

Implementation summary:

- Added a pure `MovementInputController` for bounded held-movement resend, immediate direction/stop sends, and retry after rejection.
- Added a config-driven movement reliability test that reads steps from `tests/scenarios/movement_reliability.json`.
- Wired `MainWindow` to poll held movement with a `QTimer`, reset movement state on match/disconnect transitions, retry after rejected input, and surface rejection feedback.
- Added the movement input test to CTest and the Qt-enabled build.

## Test Impact Matrix

Changed behavior:

- Qt desktop movement now resends held movement at a bounded interval and allows same-direction retry after rejected input.
- Stop is still sent immediately when all movement keys are released.
- Rejection feedback is visible in the event log and error surface.

Tests added/updated:

- Happy path: `battle_qt_client_movement_input_tests` loads `tests/scenarios/movement_reliability.json` and verifies immediate move, bounded resend, retry-after-rejection, immediate stop, and no idle resend after stop.
- Corner cases: same-direction poll before the resend interval emits no command; poll after stop emits no command.
- Invalid input / hostile input: no parser, auth, network payload, or untrusted transport boundary changed.
- Authority / ownership: the Qt client still sends intentions only and does not mutate authoritative positions, objective, score, HP, cooldowns, or match result.
- Resource bounds / performance: resend interval is bounded at 150 ms, well below the server's command-rate budget and not tied to frame rate.
- Regression: full CTest suite, Qt-enabled configure/build/CTest, architecture/no-hardcoding/secret/agent validators, and diff check.
- Manual UI checks: no live Qt window interaction was performed; the input model is covered by deterministic tests and the Qt target compiled.

Not tested and why:

- A full desktop end-to-end capture route was not automated in this task; Priority 4 adds paired config-driven gameplay scenario tests after movement reliability is in place.

## Verification-Agent

Passed.

Checks run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON`
- `cmake --build build-qt-mingw --parallel`
- `ctest --test-dir build-qt-mingw --output-on-failure`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

Notes:

- `git diff --check` reported only line-ending conversion warnings and no whitespace errors.

## Review-Agent

Architecture-Agent: approved. The input controller is client-side intention scheduling only and does not change server authority, gameplay rules, protocol DTOs, or subsystem dependencies.

Qt-Agent: approved. MainWindow uses signal/timer-driven UI flow, does not block the UI thread, and keeps networking through `NetworkClient`.

Security Review Agent: not required. No auth, parser, network input validation, session identity, or secret handling changed.

Performance Review Agent: not required. The resend timer is bounded and client-local; server-side rate limiting remains authoritative.

## Commit-Agent

Completed.

- Commit: current task commit, `qt: add reliable held movement input`
- Branch: `agent/0047-playable-stabilization`
- No merge to `master` or `main`.
- No push or release tag.

## Coordinator Closeout

Task 0049 is terminal on the stabilization branch. The untracked local plan file `post_0046_playable_stabilization_agent_plan.md` remains unstaged.
