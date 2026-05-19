# Task: Create Telegram Mini App skeleton

## Phase

Phase 8. Telegram Mini App

## Goal

Create strict TypeScript Mini App skeleton with Telegram bridge, WebSocket client, and Canvas placeholder.

## Allowed files

- `frontend/telegram_mini_app/`
- `docs/frontend/TYPESCRIPT_FRONTEND_STANDARDS.md`
- `docs/telegram/TELEGRAM_MINI_APP.md`

## Forbidden files

- `src/battle_core/`
- `src/battle_backend/` except API docs

## Required quality gates

- Gate A
- Gate E

## Security impact

medium

## Performance/scalability impact

low

## Architecture impact

medium

## Token budget instructions

- Do not inspect node_modules.
- Read only frontend standards and telegram docs.

## Implementation steps

1. Add Vite/TypeScript config.
2. Add TelegramBridge reading raw initData.
3. Add WebSocketClient skeleton.
4. Add CanvasRenderer placeholder.
5. Add touch controls placeholder.
6. Add lint/typecheck scripts.

## Required tests/checks

- `npm run typecheck` passes.
- `npm run lint` passes or placeholder config is documented.
- No secrets in frontend.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Frontend-Agent: yes
- Security-Agent: yes for Telegram bridge
- Verification-Agent: yes

## Acceptance criteria

- Scope is not broadened.
- Required checks pass or blockers are documented.
- Agent progress is updated with files changed, tests run, and remaining risks.
