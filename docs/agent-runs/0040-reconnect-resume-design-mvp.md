# Task 0040: Reconnect / resume design and MVP

## State

committed

## Coordinator Preflight

- Branch: `agent/0040-reconnect-resume-design-mvp`.
- Baseline: `master` after local merge of task 0039.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0040-reconnect-resume-design-mvp.md`.
- Allowed implementation scope confirmed: reconnect design doc, Telegram Mini App WebSocket client, focused frontend behavior test, frontend README, task docs.
- Forbidden scope confirmed: no backend/server/transport/protocol schema changes, no insecure resume token, no workflows, deploy files, release tags, push, or task 0041 work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0040. Document reconnect/resume semantics and ship the safe MVP behavior for current clients: after an in-match disconnect, show a clear unsupported-resume diagnostic and clear stale match state so reconnecting cannot send old-match commands before a fresh join. Do not implement backend resume, protocol schema changes, or frontend-only auth/session tokens.

## Implementation-Agent

- Added `docs/architecture/RECONNECT_RESUME.md` with current MVP behavior, future local-browser and Telegram resume semantics, server-side identity/slot requirements, and explicit MVP non-goals.
- Updated `WebSocketClient` close handling to clear stale `matchId` and reset `sessionSeq` when the socket closes.
- Changed in-match close diagnostics from a generic WebSocket close to `disconnected, rejoin not supported yet (...)`.
- Preserved generic close diagnostics for clients that were not in a match.
- Updated Mini App README with the current reconnect/resume behavior.
- Updated frontend WebSocket behavior test to prove stale match commands are not sent after close or after reconnect/auth before a fresh join.

## Test Impact Matrix

Changed behavior:

- In-match WebSocket close now produces a clear unsupported-resume diagnostic.
- The frontend clears local match state on close, preventing stale old-match commands after reconnect.

Tests added/updated:

- Happy path: existing auth, create, join, command, ping/pong WebSocket client behavior remains covered.
- Corner cases: updated test covers close after match join, reconnect/auth after close, and command attempt before a fresh join.
- Invalid input / hostile input: protocol validation test remains passing; no parser/auth trust boundary was loosened.
- Authority / ownership: stale client match state is cleared so the client cannot send old-match command envelopes after reconnect.
- Resource bounds / performance: no queues, retry loops, timers, or reconnect floods were introduced.
- Regression: WebSocket behavior test covers the previous passive ping/pong path and new unsupported-resume close behavior.
- Manual UI checks: none required; visible diagnostic is set through the same state path covered by the client behavior test.

Not tested and why:

- Secure backend resume is not implemented or tested in this task because it requires protocol/session/auth changes and Security Review beyond the safe MVP.
- GitHub-hosted CI was not run because no push was performed.

Ready for verification: yes.

## Verification-Agent

- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed.
- `node tests\frontend\telegram_websocket_client_behavior.mjs` - passed.
- `node tests\frontend\telegram_protocol_validation.mjs` - passed.
- `python scripts\ci\validate_architecture_boundaries.py` - passed.
- `python scripts\ci\scan_secrets.py` - passed.
- `python scripts\agent\validate_agent_harness.py` - passed.
- `python scripts\agent\validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed.
- `scripts/ci/validate_structure.sh` through Git Bash shim - passed.

## Review-Agent

- Frontend Review-Agent: approved. The UX change stays inside WebSocket client state/diagnostics and does not add UI complexity.
- Security Review-Agent: approved. No frontend-only resume token, localStorage secret, protocol schema, or backend auth shortcut was introduced.
- Architecture Review-Agent: approved. Frontend owns reconnect UX only; authoritative session/resume semantics remain documented for future backend work.
- Verification-Agent: approved. Required checks passed.

## Fix-Agent

- No review-blocking fixes required after verification.

## Commit-Agent

- Commit created with message `frontend: clarify reconnect unsupported state`.
- Final commit hash is recorded in the coordinator response because amending this file changes the hash.
- Scope checked before commit: only task 0040 files were staged; root roadmap file remained unstaged.

## Coordinator Closeout

- Task 0040 is complete on branch `agent/0040-reconnect-resume-design-mvp`.
- Final commit hash is recorded in the coordinator response.
- Ready for local merge to `master`.
- No push or release tag was performed.
