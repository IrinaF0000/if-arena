# Task 0032: Fix WebSocket client envelope and idle timeout

## Task title
`Fix WebSocket client envelope and idle timeout`

## Phase
`Post-v0.1.0 playable MVP bugfix/polish`

## Goal
Fix the local browser / Telegram Mini App WebSocket playable path so two browser clients can play one match without protocol envelope errors, passive disconnects, stale snapshots, or inconsistent screen-oriented movement.

## Observed bugs

- Two browser clients can connect, authenticate, and join the same match.
- Web `input_command` messages are sent without `"version": 1`, causing `protocol: invalid_envelope`.
- Passive clients disconnect with idle timeout while in an active match.
- Web match ticks/snapshots appear too slow or event-driven instead of live.
- Movement direction may be incorrectly transformed in the web client.

## Expected behavior

- Every outgoing web envelope includes `version: 1`.
- No `invalid_envelope` appears after join or during movement/actions.
- Two browser clients stay connected in one match.
- A passive player remains connected while receiving snapshots.
- Server validation remains strict.
- Web client does not send no-op/none spam.
- Movement buttons are screen-oriented and consistent for both players: Up moves up, Down moves down, Left moves left, Right moves right.

## Allowed files

- `frontend/telegram_mini_app/**`
- `src/battle_server_app/**` if required for WebSocket keepalive/live tick semantics
- `src/battle_backend/**` if required for shared live match behavior
- `tests/**`
- `docs/game/**` if controls/player-view docs need clarification
- `docs/agent-runs/**`
- `docs/agent-tasks/0032-fix-websocket-client-envelope-and-idle-timeout.md`

## Forbidden files

- `.github/workflows/**`
- `deploy/**`
- `external/**`
- unrelated Qt/TCP/frontend features
- release tag creation
- task 0033 or later
- weakening protocol validation
- unrelated gameplay features

## Required quality gates

- Gate A
- Gate C for client authority and network input validation
- Gate F/G for WebSocket idle/tick/load/security behavior
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity

## Sequential agent pipeline

Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.

## Implementation steps

1. Fix Telegram Mini App WebSocket protocol serialization so all outgoing envelopes, especially `input_command`, include `version: 1`.
2. Add frontend tests for auth, create, join, move, stop, attack, and dash envelope serialization.
3. Add a regression test proving all `input_command` envelopes include `version: 1`.
4. Add a regression test proving no automatic invalid/no-op command is sent after `match_joined`.
5. Fix WebSocket ping/idle handling or server-side active-match keepalive semantics so passive matched clients remain connected while receiving snapshots.
6. Fix WebSocket live tick/snapshot cadence so browser clients receive live snapshots during an active match.
7. Fix web input direction mapping so screen buttons are consistent for both players: Up moves up, Down moves down, Left moves left, Right moves right.
8. Add useful client/server diagnostics for disconnect reasons without logging secrets.

## Required tests

- Frontend protocol tests for auth, create, join, move, stop, attack, dash.
- Regression test that all `input_command` envelopes include `version: 1`.
- Regression test that no automatic invalid/no-op command is sent after `match_joined`.
- WebSocket two-client smoke: A creates match, B joins, A moves, B stays passive, both remain connected beyond idle timeout, both receive snapshots, no `invalid_envelope`.
- Existing CMake, Qt, TCP, load, security, frontend, and agent validators must still pass.

## Manual checklist

Record status in the run note:

1. Start WS server with TCP disabled.
2. Start Vite with `VITE_WS_URL=ws://127.0.0.1:8081/ws`.
3. Open two browser tabs.
4. Tab 1: Connect -> Create.
5. Tab 2: Connect -> Join.
6. Move only tab 1 for more than idle timeout.
7. Verify tab 2 stays connected and receives updates.
8. Verify movement directions are correct.
9. Verify browser console has no protocol errors.

## Required reviews

- Coordinator: yes
- Architecture-Agent: yes if server/backend live tick or keepalive behavior changes
- Security-Agent: yes for protocol validation, idle timeout, auth/session, and diagnostics
- Performance-Agent: yes for WebSocket tick/snapshot/keepalive changes
- Frontend-Agent: yes
- Verification-Agent: yes

## Rollback note

One scoped commit. Do not create a release tag. Do not continue to task 0033.
