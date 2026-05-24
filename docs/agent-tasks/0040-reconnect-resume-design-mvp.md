# Task 0040: Reconnect / resume design and MVP

## Goal

Define reconnect/resume semantics for Web/Telegram clients and ship the safe MVP behavior: when secure resume is not yet implemented, the client must fail clearly and must not reuse stale match/session state after reconnect.

## Scope

Allowed files:

- `docs/architecture/RECONNECT_RESUME.md`
- `frontend/telegram_mini_app/src/network/WebSocketClient.ts`
- `frontend/telegram_mini_app/README.md`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `docs/agent-runs/0040-reconnect-resume-design-mvp.md`
- `docs/agent-tasks/0040-reconnect-resume-design-mvp.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `src/battle_core/**`
- `src/battle_backend/**`
- `src/battle_server_app/**`
- `src/battle_transport_*`
- public protocol schema changes
- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- release tags
- pushing to GitHub

## Required behavior

- Document target reconnect/resume semantics for local browser and Telegram identities.
- Do not implement insecure frontend-only resume tokens.
- On in-match WebSocket close, show a clear diagnostic that reconnect/resume is not supported yet.
- Clear stale client match state on close so a later reconnect cannot send commands for the old match before a fresh join.
- Preserve current auth, join, ping/pong, and command behavior for normal connected sessions.

## Required quality gates

- Gate A. Every implementation task
- Gate E. Telegram Mini App
- Gate F. Security-sensitive reconnect/auth boundary
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `cd frontend/telegram_mini_app && npm.cmd run typecheck`
- `cd frontend/telegram_mini_app && npm.cmd run lint`
- `cd frontend/telegram_mini_app && npm.cmd run build`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Frontend-Agent: yes
- Security-Agent: yes
- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Reconnect/resume design is documented without promising unsupported production behavior.
- Client displays clear unsupported-resume state after in-match disconnect.
- Client clears stale match state after close and does not send old-match commands after reconnect/auth before a fresh join.
- No backend resume, protocol, or auth changes are introduced in this task.

## Rollback note

Revert this task commit to restore the prior generic close diagnostic and remove the reconnect/resume design note.
