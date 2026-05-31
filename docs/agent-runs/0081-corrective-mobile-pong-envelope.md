# Run 0081: Corrective mobile pong envelope handling

## Status

State: committed
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0081-corrective-mobile-pong-envelope.md`
Implementation commit: this commit (`fix: handle mobile pong envelopes`)

## Scope

- Telegram Mini App inbound protocol validation and focused browser/client regression tests.
- No gameplay authority changes, workflow/deploy changes, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Test Impact Matrix

Changed behavior:
- Telegram Mini App runtime validation now accepts server-originated `pong` envelopes.
- Mobile UI ignores incoming `pong` envelopes instead of replacing connection state with `protocol: invalid_envelope`.
- WebSocket launcher now prefers `build\battle_server_app.exe` and uses `build-qt-mingw\battle_server_app.exe` only as fallback.

Tests added/updated:
- Happy path: protocol validation parses incoming `ping` and `pong`; mobile e2e authenticates two clients and plays through a match.
- Corner cases: WebSocket behavior test verifies incoming `pong` does not echo traffic or create a parse diagnostic.
- Invalid input / hostile input: existing malformed JSON and invalid snapshot envelope checks remain covered.
- Authority / ownership: unchanged; frontend still sends intentions only and tests keep authority fields out of input commands.
- Resource bounds / performance: launcher smoke and e2e stop checks avoid stale local processes; no queue or tick-loop behavior changed.
- Regression: e2e asserts no `protocol:` diagnostic is visible immediately after authentication.
- Manual UI checks: launcher smoke started the WebSocket server from the current `build` executable.

Not tested and why:
- Full manual phone/Telegram container flow was not run; local browser e2e covers the WebSocket/mobile UI path without real Telegram credentials.

## Checks

- `node tests\frontend\telegram_protocol_validation.mjs` - passed.
- `node tests\frontend\telegram_websocket_client_behavior.mjs` - passed.
- `npm.cmd run typecheck` in `frontend/telegram_mini_app` - passed.
- `npm.cmd run lint` in `frontend/telegram_mini_app` - passed.
- `npm.cmd run build` in `frontend/telegram_mini_app` - sandbox blocked Vite config read; rerun with escalation passed.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed before e2e.
- `npm.cmd run test:e2e` in `frontend/telegram_mini_app` - sandbox blocked Vite webServer config read; rerun with escalation passed.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed after e2e.
- `cmd /c "echo. | play_if_arena_ws.cmd"` - passed with escalation.
- `Get-Process battle_server_app,node ...` - confirmed `battle_server_app` ran from `build\battle_server_app.exe`.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed after launcher smoke.
- `python scripts\agent\validate_agent_harness.py` - passed.
- `python scripts\agent\validate_playable_task_sequence.py` - passed.
- `python scripts\ci\scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warnings only.

## Reviews

- Verification-Agent: approved; checks passed and generated Playwright `test-results/` was removed.
- Frontend-Agent: approved; runtime validator and UI message handling are explicit and scoped.
- Security-Agent: approved; inbound network message validation remains deny-by-default, with only the known `pong` envelope added.

## Risks

- A user with an already-running stale Vite or server process can still see old behavior until `scripts\run\stop_if_arena.cmd` is run or the launcher restarts the stack.
