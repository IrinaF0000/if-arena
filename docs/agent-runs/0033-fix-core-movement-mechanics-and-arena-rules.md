# Task 0033: Fix core movement mechanics and playable arena rules

## State

commit-ready

## Coordinator Preflight

- Branch: `agent/0033-fix-core-movement-mechanics-and-arena-rules`.
- Baseline: `58aaf86 add cmd to start game`, on top of `0616b00 websocket: stabilize browser playable path`.
- Working tree at preflight had one untracked local file not owned by this task: `config/examples/server.ws.local.json`. It is not staged or modified by this task unless explicitly needed.
- Task packet created: `docs/agent-tasks/0033-fix-core-movement-mechanics-and-arena-rules.md`.
- Allowed implementation scope confirmed: `src/battle_core/**`, `src/battle_backend/**`, `src/battle_server_app/**` only if needed, `src/battle_qt_client/**`, `frontend/telegram_mini_app/**`, `tests/**`, `docs/game/**`, `docs/agent-runs/**`, and the task packet itself.
- Forbidden scope confirmed: no `.github/workflows/**`, `scripts/ci/**`, `deploy/**`, `external/**`, release tag, task 0034, unrelated gameplay/networking/deployment/UI features, or client-side authoritative rules.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0033. Fix the playable Objective Run mechanics so movement is bottom-to-top, screen-consistent, single-step/configured-speed, and not blocked directly from spawn or back to base. Make Space attack in Qt and never disconnect; keep mouse movement as aim-only. Make objective pickup/capture automatic and visibly readable. Add config-driven default contest pressure with obstacles/hazards without putting authoritative gameplay in clients. Add/update required tests and record a Test Impact Matrix before verification. Do not create a release tag or start task 0034.

## Implementation-Agent

Assumptions:

- Backend command directions remain team-local intent; `battle_backend` is still the only layer that maps session/team intent into canonical `battle_core` directions.
- Bottom-to-top means each active player sees their own base at the bottom; Red clients rotate rendering locally instead of asking the server to change canonical coordinates.
- Default contest pressure should not block the direct spawn/objective/base lane, so hazards and side obstacles pressure alternate routes and the objective area without overlapping spawns or the objective spawn.

Changed files:

- `docs/agent-tasks/0033-fix-core-movement-mechanics-and-arena-rules.md`
- `docs/agent-runs/0033-fix-core-movement-mechanics-and-arena-rules.md`
- `docs/game/MAP_AND_FAIRNESS.md`
- `frontend/telegram_mini_app/README.md`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/src/network/WebSocketClient.ts`
- `frontend/telegram_mini_app/src/style.css`
- `frontend/telegram_mini_app/src/ui/TouchControls.ts`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/README.md`
- `src/battle_core/ArenaConfig.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/README.md`
- `src/battle_qt_client/tests/CoordinateTransformTests.cpp`
- `src/battle_qt_client/ui/ArenaView.cpp`
- `src/battle_qt_client/ui/MainWindow.cpp`
- `src/battle_qt_client/ui/MainWindow.hpp`
- `src/battle_qt_client/ui/README.md`
- `tests/frontend/telegram_touch_controls_behavior.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/unit/core/BattleEngineFacadeTests.cpp`

Implementation notes:

- Opened the default canonical vertical lane by removing center-lane obstacles at `10,3` and `10,9`, then added symmetric side obstacles and symmetric hazard configs to the core arena config.
- Propagated default hazards into backend-created playable matches and tuned backend playable speed to `0.25` cells per tick for live 20 Hz play.
- Added automatic objective pickup on contact in `BattleEngine`; capture remains automatic when the carrier reaches own base.
- Expanded backend event batches from opaque counts to readable event records so pickup/drop/capture/hit/dash feedback can be surfaced by clients.
- Fixed Web/Mini App direction contract: clients send team-local intentions; rendering rotates for Red so Up/Down remain screen-consistent in the local player view.
- Updated Web controls so Space/Attack and Shift/Dash use current or last non-zero direction, Stop sends no direction, and no interact/no-op spam is emitted.
- Fixed Qt keyboard focus behavior so Space attacks during a match instead of activating focused buttons, while mouse movement remains aim-only. Enlarged Qt and Web arena surfaces and improved objective/carrier feedback.

Checks run:

- `node tests\frontend\telegram_protocol_validation.mjs` - pass.
- `node tests\frontend\telegram_websocket_client_behavior.mjs` - pass.
- `node tests\frontend\telegram_touch_controls_behavior.mjs` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass.
- `npm.cmd run typecheck` - pass.
- `npm.cmd run lint` - pass.
- `npm.cmd run build` - pass with escalation because sandboxed esbuild could not read `vite.config.ts`.
- `python tests\frontend\websocket_local_smoke.py` - pass.
- `python tests\integration\server\tcp_vertical_slice_smoke.py` - pass.
- `python tests\integration\server\tcp_live_tick_smoke.py` - pass.
- `python tests\security\tcp_protocol_negative.py` - pass.
- `python tests\load\load_client_dry_run.py` - pass.
- `python tests\load\local_tcp_load_scenarios.py --report "$env:TEMP\if_arena_0033_local_tcp_smoke.md"` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake --build build-qt-mingw --parallel` - failed once on a heterogeneous Qt widget initializer list, fixed by using `std::array<QWidget*, 7>`, then pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; ctest --test-dir build-qt-mingw --output-on-failure` - pass.
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `python scripts\ci\scan_secrets.py` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- `$env:Path = "C:\Program Files\Git\usr\bin;C:\Program Files\Git\bin;$env:Path"; & "C:\Program Files\Git\bin\bash.exe" -lc 'python3() { /c/Python314/python.exe "$@"; }; export -f python3; ./scripts/ci/validate_structure.sh'` - pass.
- `git diff --check` - pass.

Ready for verification: yes.

## Test Impact Matrix

Changed behavior:

- Direct vertical movement from each spawn/base toward the objective is open and bottom-to-top.
- Backend playable movement speed is smooth and no longer jumps logical cells at live tick cadence.
- Objective pickup happens automatically on contact; capture still happens automatically in own base.
- Default playable arena contains symmetric config-driven hazards and side obstacles without blocking spawns/objective.
- Web client sends team-local directions and rotates rendering for Red.
- Web attack/dash use the last meaningful direction; Stop sends no direction; interact button/key was removed from Mini App controls.
- Qt Space attacks during a match and buttons no longer steal Space into Disconnect/Create/Join actions.
- Event batches contain readable event records for pickup/drop/capture/hit/dash feedback.

Tests added/updated:

- Happy path: core tests for Blue direct spawn movement, Red direct spawn movement, carrier return/capture, auto pickup, and default hazards; backend fake-session full match updated for direct lane and automatic pickup.
- Corner cases: smooth speed test verifies first live-speed tick does not skip a logical cell; Qt transform tests cover lateral transforms; Web controls test covers last-direction fallback.
- Invalid input / hostile input: existing protocol negative/security tests still pass; command validation remains server-side.
- Authority / ownership: clients still send intentions only; backend still maps session-owned player/team to core commands.
- Resource bounds / performance: TCP load dry-run and local load scenario still pass; hazards are bounded config data and event batches remain per tick.
- Regression: Space/Attack does not emit disconnect/close in Web control test; WebSocket client no longer double-inverts Red direction; Qt MinGW build verifies the focus/intercept implementation compiles with Qt.
- Manual UI checks: checklist below is recorded; full two-Qt-client manual play was not executed in this non-interactive session.

Not tested and why:

- Full manual two-Qt-client checklist was not executed because this session cannot drive two interactive Qt windows. Automated core/backend/Qt compile/transform/frontend/transport smokes cover the protocol and mechanics paths.

## Verification-Agent

Result: pass after one Fix-Agent iteration for Qt MinGW compile.

Verified diff scope contains no forbidden paths. Existing untracked `config/examples/server.ws.local.json` remains unstaged and untouched.

Required gates passed:

- CMake configure/build/CTest.
- Qt MinGW configure/build/CTest.
- Frontend protocol/client/control tests.
- Frontend typecheck/lint/build.
- TCP, WebSocket, load, and security smoke/negative checks.
- Secret scan, structure validator, agent harness validator, playable sequence validator, and diff whitespace check.

## Review-Agent

Decision: approve.

Architecture-Agent:

- Core owns movement, hazards, pickup, capture, combat/drop rules.
- Backend owns session/team-to-core direction mapping and match config assembly.
- Qt/Web clients render local player orientation and send intentions only; no client-owned state authority was added.

Security-Agent:

- No protocol validation weakening.
- Client changes do not trust Telegram `initDataUnsafe` or add secrets.
- Event diagnostics expose typed gameplay events only, not raw input payloads or auth material.

Performance-Agent:

- Backend speed tuning is constant config, not extra per-session work.
- Hazards are bounded match config vectors validated by core arena rules.
- Event batch detail grows with deterministic per-tick event count; existing load checks passed.

Qt-Agent:

- Space/Shift are routed as gameplay keys during matches.
- Buttons no longer capture Space focus for disconnect/create/join.
- Mouse movement remains aim-only; click actions send intentions through `NetworkClient`.
- Arena surface is larger and base coloring follows local view.

Frontend-Agent:

- TypeScript remains strict.
- Mini App controls use last non-zero direction and avoid idle/no-op/interact spam.
- Red rendering rotates locally while outbound commands remain team-local.
- Objective/carrier/event feedback is more visible.

## Fix-Agent

Fixed one verification finding:

- Qt MinGW rejected heterogeneous widget pointer initializer-list deduction in `MainWindow.cpp`; replaced it with explicit `std::array<QWidget*, 7>`.

Re-checks:

- Qt MinGW build - pass.
- Qt MinGW CTest - pass.

## Commit-Agent

Pending commit.

## Coordinator Closeout

Pending commit and final status. No release tag created and no next task started.
