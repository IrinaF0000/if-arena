# Task 0081: Corrective mobile pong envelope handling

## Goal

Stop the Telegram Mini App from showing `protocol: invalid_envelope` when the WebSocket backend sends a valid protocol `pong` envelope.

## Scope

Allowed files:

- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/main.ts`
- `play_if_arena_ws.cmd`
- `tests/frontend/telegram_protocol_validation.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `frontend/telegram_mini_app/tests/e2e/two-client-websocket.spec.ts`
- `docs/agent-runs/0081-corrective-mobile-pong-envelope.md`
- `docs/agent-tasks/0081-corrective-mobile-pong-envelope.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- `battle_core` gameplay rules
- generated build outputs
- generated dependency directories
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Frontend runtime validation accepts server-originated `pong` envelopes.
- The mobile UI ignores incoming `pong` envelopes instead of replacing connection state with a protocol error.
- WebSocket launcher uses the current `build\battle_server_app.exe` before falling back to stale/alternate build trees.
- Browser E2E covers the post-connect state so this regression is visible before manual testing.

## Required checks

- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `cd frontend/telegram_mini_app && npm run typecheck`
- `cd frontend/telegram_mini_app && npm run lint`
- `cd frontend/telegram_mini_app && npm run build`
- `cd frontend/telegram_mini_app && npm run test:e2e`
- `cmd /c scripts\run\stop_if_arena.cmd`
- `cmd /c "echo. | play_if_arena_ws.cmd"`
- `cmd /c scripts\run\stop_if_arena.cmd`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Verification-Agent: yes
- Frontend-Agent: yes
- Security-Agent: yes
- Architecture-Agent: no
- Performance-Agent: no

## Acceptance criteria

- A valid incoming `{ "version": 1, "type": "pong", "payload": {} }` parses as a server message.
- WebSocket client behavior tests prove incoming `pong` does not emit `client_parse_error`.
- Launcher smoke proves the WebSocket server starts from the preferred current `build` executable when present.
- Mobile E2E asserts no `protocol:` diagnostic is shown immediately after authentication.

## Rollback note

Revert this task commit to restore the previous frontend parser behavior.
