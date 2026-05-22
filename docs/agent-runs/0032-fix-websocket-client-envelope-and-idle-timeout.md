# Task 0032: Fix WebSocket Client Envelope And Idle Timeout

## State

commit-ready

## Coordinator Preflight

- Branch: `agent/0032-fix-websocket-client-envelope-and-idle-timeout`.
- Baseline: `71c300d Merge task 0031 playable controls and readability`.
- Working tree at preflight had untracked local files not owned by this task: `config/examples/server.ws.local.json`, `frontend/telegram_mini_app/package-lock.json`, and `play_if_arena.cmd`. `frontend/telegram_mini_app/package-lock.json` was later deleted after explicit user authorization because it blocked repo secret/structure checks; the other untracked files remain unstaged and untouched.
- Task packet created: `docs/agent-tasks/0032-fix-websocket-client-envelope-and-idle-timeout.md`.
- Allowed implementation scope confirmed: `frontend/telegram_mini_app/**`, `src/battle_server_app/**` if needed for WebSocket keepalive/live tick semantics, `src/battle_backend/**` if needed for shared live match behavior, `tests/**`, `docs/game/**`, `docs/agent-runs/**`, and the task packet itself.
- Forbidden scope confirmed: no `.github/workflows/**`, `deploy/**`, `external/**`, release tag, task 0033, protocol validation weakening, or unrelated gameplay features.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Review-Agent -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0032. Fix the Telegram Mini App WebSocket playable path so browser clients serialize every envelope with `version: 1`, do not send invalid/no-op input after join, stay connected passively in active matches, receive live snapshots, and use screen-oriented directions consistently. Add the required frontend protocol tests and a WebSocket two-client smoke. Keep server validation strict and do not add unrelated gameplay features or create a release tag. Record checks and a Test Impact Matrix here before verification.

## Implementation-Agent

Assumptions:

- WebSocket/browser controls render in canonical screen orientation for this MVP, so screen Up/Down/Left/Right should map to canonical visual movement for both teams.
- The backend still expects team-local movement intent and transforms it at the session boundary; therefore the browser client inverts directions for Red after learning the local team from `match_joined` or snapshot.
- Application-level server `ping` envelopes should be answered by the Mini App with protocol `pong` envelopes; browser WebSocket control frames are not enough because the server uses protocol messages for lifecycle checks.

Changed files:

- `docs/agent-tasks/0032-fix-websocket-client-envelope-and-idle-timeout.md`
- `docs/agent-runs/0032-fix-websocket-client-envelope-and-idle-timeout.md`
- `frontend/telegram_mini_app/README.md`
- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/src/network/WebSocketClient.ts`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `src/battle_server_app/main.cpp`
- `tests/frontend/telegram_protocol_validation.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `tests/frontend/websocket_local_smoke.py`

Implementation notes:

- Added strict frontend parsing for server `ping` and frontend serialization for `pong`, all with `version: 1`.
- WebSocketClient now responds to `ping`, reports close/error diagnostics, stores session/team state, and maps Red screen directions through the inverse team transform before sending commands.
- WebSocket `match_joined` responses now include `team` for the browser path so input direction mapping is correct before the first snapshot.
- WebSocket server runtime now advances active matches on tick timeouts, matching the TCP live-tick behavior from task 0031.
- WebSocket input no longer forces a tick per accepted command; accepted commands are applied by the due live tick.
- WebSocket smoke now exercises two clients with A active and B passive beyond idle timeout, while both receive snapshots and answer pings.

Checks run:

- `node tests\frontend\telegram_protocol_validation.mjs` - pass.
- `node tests\frontend\telegram_websocket_client_behavior.mjs` - pass.
- `cmake --build build --parallel` - pass.
- `python tests\frontend\websocket_local_smoke.py` - pass.
- `npm.cmd run typecheck` - pass.
- `npm.cmd run lint` - pass.
- `npm.cmd run build` - pass with escalation because sandboxed esbuild could not read `vite.config.ts`.
- `ctest --test-dir build --output-on-failure` - pass.
- `python tests\integration\server\tcp_vertical_slice_smoke.py` - pass.
- `python tests\integration\server\tcp_live_tick_smoke.py` - pass.
- `python tests\load\load_client_dry_run.py` - pass.
- `python tests\load\local_tcp_load_scenarios.py --report "$env:TEMP\if_arena_0032_local_tcp_smoke.md"` - pass.
- `python tests\security\tcp_protocol_negative.py` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake --build build-qt-mingw --parallel` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; ctest --test-dir build-qt-mingw --output-on-failure` - pass.
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `git diff --check` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- `python scripts\ci\scan_secrets.py` - pass after authorized cleanup of pre-existing untracked `frontend\telegram_mini_app\package-lock.json`.
- `$env:Path = "C:\Program Files\Git\usr\bin;C:\Program Files\Git\bin;$env:Path"; & "C:\Program Files\Git\bin\bash.exe" -lc 'python3() { /c/Python314/python.exe "$@"; }; export -f python3; ./scripts/ci/validate_structure.sh'` - pass after authorized cleanup of pre-existing untracked `frontend\telegram_mini_app\package-lock.json`.

Ready for verification: yes.

## Test Impact Matrix

Changed behavior:

- Browser envelopes, including every `input_command`, serialize with `version: 1`.
- Browser client replies to server protocol `ping` with versioned `pong`.
- Browser client does not send automatic no-op/none commands after `match_joined`.
- Passive WebSocket clients stay connected beyond idle timeout while receiving active match snapshots.
- WebSocket active matches tick live instead of only on join/input events.
- Browser screen directions remain visually consistent for both teams.
- WebSocket close/error diagnostics are surfaced in the client and timeout/protocol diagnostics are logged server-side.

Tests added/updated:

- Happy path: `tests/frontend/websocket_local_smoke.py` now covers two WS clients, create/join, A movement, passive B beyond idle timeout, snapshots for both, ping/pong, and no server protocol error.
- Corner cases: `tests/frontend/telegram_websocket_client_behavior.mjs` covers no automatic input after `match_joined` and Red pre-snapshot direction mapping from `team`.
- Invalid input / hostile input: existing `tests/security/tcp_protocol_negative.py`, protocol tests, and WebSocket transport tests still pass; protocol validation remains strict.
- Authority / ownership: frontend still sends only command kind/direction and never state fields; protocol validation test asserts no HP authority claim in input command.
- Resource bounds / performance: WebSocket tick catch-up is bounded to four ticks per poll cycle; local load smoke still passes.
- Regression: frontend protocol test covers auth/create/join/move/stop/attack/dash/pong serialization and `input_command` `version: 1`.
- Manual UI checks: checklist from task packet recorded below; full browser two-tab manual play not executed in this non-interactive session.

Not tested and why:

- Full Vite browser two-tab manual checklist was not executed because this agent session is non-interactive. The local raw WebSocket smoke exercises the same backend protocol path and passive idle behavior without a browser UI.

## Manual Checklist Status

- Start WS server with TCP disabled: covered by `tests/frontend/websocket_local_smoke.py`.
- Start Vite with `VITE_WS_URL=ws://127.0.0.1:8081/ws`: not manually run.
- Open two browser tabs: not manually run.
- Tab 1 Connect/Create and Tab 2 Connect/Join: protocol-equivalent smoke passed.
- Move only tab 1 for more than idle timeout: protocol-equivalent smoke passed.
- Verify tab 2 stays connected and receives updates: protocol-equivalent smoke passed.
- Verify movement directions are correct: covered by frontend behavior test for Red direction inversion and existing controls mapping.
- Verify browser console has no protocol errors: not manually run; frontend protocol/client tests and WS smoke show no `invalid_envelope`.

## Verification-Agent

Result: pass.

Verification coverage:

- Frontend protocol/client behavior: passed `telegram_protocol_validation.mjs` and `telegram_websocket_client_behavior.mjs`.
- WebSocket playable path: passed two-client local smoke with create/join, active creator movement, passive joiner beyond idle timeout, ping/pong, snapshots for both clients, and no server protocol error.
- Existing CMake/tests: passed default CMake build, default CTest, Qt MinGW configure/build/CTest, TCP vertical slice, TCP live tick, TCP load, and TCP security negative checks.
- Frontend build health: passed typecheck, lint, and production build.
- Repo hygiene: passed diff whitespace check, agent validators, secret scan, and Git Bash structure validation.

Manual browser two-tab UI checklist was not run; the protocol-equivalent local smoke covers the backend WebSocket behavior and the frontend behavior test covers browser serialization, ping/pong, no no-op after join, and direction mapping.

## Review-Agent

Result: approve.

Architecture review:

- WebSocket live tick state remains in `battle_server_app`; no TCP, WebSocket, Qt, or Telegram concerns were moved into `battle_core`.
- Clients still send intentions only; authoritative state remains server-side.
- `match_joined.team` is a transport convenience for view/input mapping and does not grant client authority.

Security review:

- Protocol validation stays strict; the fix adds versioned client envelopes and does not relax server envelope parsing.
- Ping/pong handling uses normal protocol envelopes with `version: 1`.
- Diagnostics log close/protocol reasons and session ids only; no payloads, auth tokens, or secrets are logged.
- Secret scan passed after authorized removal of the unrelated untracked lockfile blocker.

Performance review:

- Active WebSocket match catch-up is bounded to four ticks per poll cycle.
- Socket poll timeout now includes tick cadence, so passive active-match clients receive periodic snapshots without busy looping.
- Local TCP load checks still pass.

Frontend review:

- TypeScript remains strict and typed; no `any`-based protocol bypass was added.
- Browser commands are versioned for auth/create/join/move/stop/attack/dash/pong.
- `match_joined` no longer causes an automatic input/no-op send.
- Screen-oriented direction mapping is covered for Red before and after snapshots.

## Fix-Agent

Not needed. Review found no blocking issues.

## Commit-Agent

Scoped files selected for commit. Pre-existing untracked `config/examples/server.ws.local.json` and `play_if_arena.cmd` are intentionally left unstaged.

## Coordinator Closeout

Task 0032 is complete pending commit creation and final response. No release tag created and task 0033 was not started.
