# Task 0073: Corrective mobile protocol state

## Goal

Fix the real Telegram Mini App/browser connect/create/join state machine so wrong-phase UI actions do not produce backend protocol errors or user-visible invalid protocol states.

## Scope

Allowed files:

- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/src/network/WebSocketClient.ts`
- `frontend/telegram_mini_app/tests/e2e/**`
- `tests/frontend/**`
- `docs/agent-runs/0073-corrective-mobile-protocol-state.md`
- `docs/agent-tasks/0073-corrective-mobile-protocol-state.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- C++ gameplay/backend authority changes
- protocol/schema changes
- scenario config changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Connect is enabled only when disconnected, closed, or errored.
- Create and Join are disabled until authenticated.
- Join code input is disabled until authenticated.
- Gameplay command buttons are disabled until in match.
- Non-auth messages are not queued before the session phase allows them.
- Wrong-phase create/join/input/rematch attempts fail locally with a clear diagnostic instead of producing server `protocol_error`.
- Local browser auth continues to use demo auth when Telegram `initData` is absent.
- Real browser e2e covers connect/create/join/snapshot and asserts no invalid protocol path.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- TypeScript/frontend quality rules
- Security review for client phase/input gating

## Required checks

- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_main_layout_contract.mjs`
- `node tests/frontend/telegram_touch_controls_behavior.mjs`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `npm.cmd run test:e2e`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: no
- Security-Agent: yes
- Frontend-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Real Playwright browser flow passes through Connect -> authenticated -> Create/Join -> snapshots.
- No `protocol_error`, `client_parse_error`, or "invalid protocol" appears in the browser e2e flow.
- No source authority moves to the client; clients still send intentions only.
- Required checks pass or any skipped check is explicitly justified.

## Rollback note

Revert this task commit to return Mini App controls and WebSocket send behavior to the previous permissive state.
