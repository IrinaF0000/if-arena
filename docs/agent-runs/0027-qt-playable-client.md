# Task 0027: Qt Playable Client

State: committed

## Coordinator Preflight

- Branch: `agent/0027-qt-playable-client`
- Base: clean `master` after task 0026 merge.
- Task packet: `docs/agent-tasks/0027-qt-playable-client.md`
- Allowed files:
  - `src/battle_qt_client/**`
  - `CMakeLists.txt`
  - `tests/manual/qt_client_checklist.md`
  - `docs/game/CONTROLS_AND_UI.md`
  - `docs/game/PLAYER_ORIENTED_VIEW.md`
  - `README.md`
  - `docs/agent-runs/`
- Forbidden files:
  - `src/battle_core/**`
  - `src/battle_backend/**` unless protocol integration blocks client work
  - `src/battle_transport_ws/**`
  - `frontend/**`
  - `deploy/**`
  - `external/**`
- Required reviews: Architecture-Agent, Qt-Agent, Verification-Agent.
- Required gates: A, D, C for client authority checks, K, L.

## Implementation Assumptions

- The Qt client uses raw TCP length-prefixed JSON envelopes and links to `battle_protocol`, not `battle_core`.
- The current backend interprets command directions as team-local intent and transforms them server-side; the Qt client sends no authoritative position, HP, score, team, or objective state.
- Qt dependency discovery is enabled only when `BATTLE_BUILD_QT_CLIENT=ON`, preserving default non-Qt CI builds.

## Implementation Note

Changed files:

- `CMakeLists.txt`
- `src/battle_qt_client/QtClientMain.cpp`
- `src/battle_qt_client/game/ClientTypes.*`
- `src/battle_qt_client/game/CoordinateTransform.*`
- `src/battle_qt_client/game/ProtocolJson.*`
- `src/battle_qt_client/network/NetworkClient.*`
- `src/battle_qt_client/ui/ArenaView.*`
- `src/battle_qt_client/ui/MainWindow.*`
- `src/battle_qt_client/tests/CoordinateTransformTests.cpp`
- `src/battle_qt_client/README.md`
- `src/battle_qt_client/network/README.md`
- `src/battle_qt_client/ui/README.md`
- `tests/manual/qt_client_checklist.md`
- `docs/game/CONTROLS_AND_UI.md`
- `docs/game/PLAYER_ORIENTED_VIEW.md`
- `README.md`

Summary:

- Replaced the placeholder Qt executable with a Qt Widgets client gated behind `BATTLE_BUILD_QT_CLIENT=ON`.
- Added asynchronous `QTcpSocket` networking outside widgets, including length-prefixed frame parsing, bounded inbound buffers, protocol envelope validation, demo auth, create/join, input commands, ping/pong, and user-facing errors.
- Added player-oriented rendering of authoritative snapshots: grid, base bands, players, HP bars, objective, hazards, HUD, and event log.
- Added keyboard/mouse controls for move, aim, attack, dash, interact, stop, and disconnect.
- Added pure coordinate transform helpers and a non-Qt CTest target.
- Updated manual checklist and docs.

Known risks:

- Qt SDK is not installed in this local environment, so the Qt-enabled target could not be compiled or manually launched here. `cmake -S . -B build-qt-ninja -G Ninja -DBATTLE_BUILD_QT_CLIENT=ON -DBATTLE_BUILD_TESTS=ON` reaches Qt package discovery and fails because no `Qt6Config.cmake` or `Qt5Config.cmake` is present.
- The local backend currently accepts team-local command directions and transforms them server-side. The Qt client follows the current playable TCP/Telegram behavior and sends no authoritative state.

Ready for verification: yes.

## Test Impact Matrix

Changed behavior:

- Qt client target is now a real Qt Widgets/Network executable when Qt is installed.
- Qt network layer opens raw TCP asynchronously, frames JSON envelopes, validates server messages, and sends only intentions.
- Qt UI renders authoritative snapshots and displays connection, lobby, HUD, errors, event log, and input feedback.
- Player-oriented coordinate helpers transform canonical snapshots for local view.

Tests added/updated:

- Happy path: `battle_qt_client_transform_tests` covers blue/red view transforms and local forward mapping.
- Corner cases: transform test covers center objective stability and hazard position transform.
- Invalid input / hostile input: `NetworkClient` enforces inbound frame length/buffer limits and rejects invalid/server-wrong envelope types; covered by existing protocol/transport tests plus manual checklist.
- Authority / ownership: transform test and manual checklist confirm no client state fields are sent; network payload builder sends `kind` and optional `direction` only.
- Resource bounds / performance: network buffer and frame bounds are enforced; UI networking uses `QTcpSocket` signals and no blocking waits.
- Regression: full default CTest suite still passes.
- Manual UI checks: checklist updated with exact connection, lobby, rendering, controls, responsiveness, and failure steps.

Not tested and why:

- Qt executable build/manual launch not run because this environment has no Qt SDK/package config (`Qt6Config.cmake`/`Qt5Config.cmake`) and no `qmake`.
- Two-client Scenario E manual pass remains for a machine with Qt installed.

## Verification

Commands run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass, 7/7 tests.
- `cmake -S . -B build-qt-ninja -G Ninja -DBATTLE_BUILD_QT_CLIENT=ON -DBATTLE_BUILD_TESTS=ON` - environment failure: Qt package config not found.
- `Get-Command qmake -ErrorAction SilentlyContinue` - no qmake found.
- `git diff --check` - pass.
- `python scripts/ci/scan_secrets.py` - pass.
- `python scripts/agent/validate_agent_harness.py` - pass.
- `python scripts/agent/validate_playable_task_sequence.py` - pass.
- `./scripts/ci/validate_structure.sh` through Git Bash with bundled Python shim - pass.

## Reviews

Architecture-Agent:

Decision: approve-with-minors

Findings:

- [minor] Qt build/manual verification requires an installed Qt SDK; target discovery is correctly gated by `BATTLE_BUILD_QT_CLIENT=ON`.
- Dependency boundary is preserved: Qt target links `battle_protocol` and Qt Widgets/Network, not `battle_core`.
- Network, rendering, and widgets are separated.

Required re-checks:

- Run Qt-enabled build and manual checklist on a Qt-equipped workstation before claiming Scenario E fully passed.

Qt-Agent:

Decision: approve-with-minors

Findings:

- [minor] Manual visual QA is pending because Qt is unavailable locally.
- No blocking socket waits are used on the UI thread.
- UI exposes connection/lobby/errors/HUD and sends intentions only.

Required re-checks:

- Execute `tests/manual/qt_client_checklist.md` after Qt SDK installation.

Verification-Agent:

Decision: approve-with-minors

Findings:

- [minor] Default build/test gates pass; Qt target package discovery fails only because Qt SDK is not installed.
- Scope is within task allowed files.
- Test Impact Matrix is present.

Required re-checks:

- Qt SDK build/manual checklist remains an environment follow-up.

## Commit

Scoped task commit created by Commit-Agent. Final hash is recorded in coordinator closeout.
