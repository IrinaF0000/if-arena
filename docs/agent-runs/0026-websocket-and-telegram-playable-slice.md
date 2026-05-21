# Task 0026: WebSocket And Telegram Playable Slice

## Status

Implemented on branch `agent/0026-websocket-and-telegram-playable-slice`.

Pipeline state: `commit-ready`.

## Changed files

- `src/battle_transport_ws/WebSocketSession.hpp`
- `src/battle_transport_ws/WebSocketSession.cpp`
- `src/battle_transport_ws/README.md`
- `src/battle_server_app/main.cpp`
- `src/battle_server_app/README.md`
- `CMakeLists.txt`
- `src/battle_backend/BackendFoundation.cpp`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/network/WebSocketClient.ts`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `frontend/telegram_mini_app/src/ui/TouchControls.ts`
- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/src/style.css`
- `frontend/telegram_mini_app/README.md`
- `tests/unit/transport/WebSocketSessionTests.cpp`
- `tests/frontend/telegram_protocol_validation.mjs`
- `tests/frontend/websocket_local_smoke.py`
- `docs/telegram/TELEGRAM_MINI_APP.md`
- `docs/telegram/TELEGRAM_SECURITY.md`
- `README.md`
- `docs/agent-runs/0026-websocket-and-telegram-playable-slice.md`

## Implementation notes

- Strengthened `WebSocketSessionAdapter` with shared protocol parsing, phase validation, fail-closed malformed/unknown/out-of-order handling, handshake timeout, idle ping/timeout checks, and bounded outbound storage.
- Added local WebSocket handshake/frame/socket support and wired WS-only server mode through backend sessions.
- Enriched backend snapshot envelopes with map, players, objective, scores, and hazards so browser clients can render authoritative state instead of placeholders.
- Fixed the Mini App WebSocket flow so auth is queued and sent only after `open`.
- Added strict runtime guards for inbound server messages and kept outbound messages to intention-only protocol envelopes.
- Added Canvas rendering for authoritative snapshots plus desktop/touch controls for move, attack, dash, interact, and stop.
- Kept Telegram auth authority on the backend boundary: frontend sends raw `initData`, never `initDataUnsafe`, and contains no secrets.

## Test Impact Matrix

Changed behavior:
- WebSocket session adapter validates message order and closes abusive sessions.
- Mini App validates inbound messages before rendering and queues auth until WebSocket open.
- Mini App renders authoritative snapshot payloads and sends player intentions only.
- Backend snapshot payloads now include renderable authoritative state.
- `battle_server_app` starts WebSocket when TCP is disabled and WebSocket is enabled in config.

Tests added/updated:
- Happy path: updated `battle_transport_ws_tests` for auth, create, and input phases; `tests/frontend/websocket_local_smoke.py` authenticates two local WS clients, creates/joins, receives a snapshot, and submits an accepted input.
- Corner cases: timeout tests cover handshake timeout, idle ping, and idle close.
- Invalid input / hostile input: WS tests cover oversized message, malformed JSON, unknown type, and gameplay before auth; frontend protocol test covers malformed JSON and invalid snapshot shape.
- Authority / ownership: frontend protocol test asserts input command payloads do not include HP/state authority fields; backend ownership tests remain unchanged.
- Resource bounds / performance: WS tests cover outbound queue/message bounds and timeout behavior.
- Regression: `ctest`, frontend typecheck/lint/build, secret scan, harness validation, and playable sequence validation.
- Manual UI checks: not run in browser in this pass.

Not tested and why:
- A real browser-to-C++ WebSocket socket smoke is not present yet because this pass keeps the C++ WebSocket code socket-library-neutral; the adapter boundary is ready for a concrete endpoint.
- Telegram replay protection remains documented as pre-production hardening; existing Telegram tests cover valid, invalid hash, stale, oversized, and redaction cases.

## Checks run

- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `python tests/frontend/websocket_local_smoke.py`
- `git diff --check`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh` via Git Bash with a temporary `python3` function pointing to bundled Python.

Browser check:
- Attempted in-app browser navigation to `http://127.0.0.1:5173/` and `http://localhost:5173`; browser runtime blocked both with `ERR_BLOCKED_BY_CLIENT`.
- Frontend was still verified by `npm.cmd run build`, strict TS/lint, and protocol/runtime smoke tests.

## Required reviews

- Architecture-Agent: approve-with-minors.
  - WebSocket transport remains outside `battle_core`.
  - Server authority remains in backend/session/match manager.
  - `CMakeLists.txt` was changed for required Windows Winsock linkage even though the task packet did not list it; accepted as a build-system necessity for this platform.
- Security-Agent: approve.
  - Frontend sends raw `initData`, never trusts `initDataUnsafe`, and contains no secrets.
  - Backend validates Telegram auth mode through backend Telegram validator before marking WS auth complete.
  - Malformed, unknown, oversized, and phase-invalid WS messages fail closed.
  - Secret scan passed.
- Performance-Agent: approve-with-minors.
  - WS message size, pending message/byte bounds, handshake timeout, idle timeout, and backend queue bounds are explicit.
  - Current local endpoint uses blocking per-client threads; production async/load hardening remains task 0028 scope.
- Frontend-Agent: approve.
  - Strict TS, lint, and build pass.
  - Runtime validation guards inbound messages before rendering.
  - Canvas renders server snapshots and controls send intentions only.
- Verification-Agent: approve.
  - Required checks above passed.
  - Test Impact Matrix is present.
  - No forbidden core, Qt, deploy, or external paths changed.

## Known risks

- The server currently runs TCP by default when both TCP and WebSocket are enabled; use a WS-only config for Mini App local development.
- Frontend rendering depends on the enriched snapshot payload shape added here.
