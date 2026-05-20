# Task 0026: Complete WebSocket and Telegram playable slice

## Task title
`Complete WebSocket and Telegram playable slice`

## Phase
`Phase 7/8 - WebSocket and Telegram Mini App`

## Goal
Implement the browser/WebView path: WebSocket gateway connected to backend plus Telegram Mini App canvas client that can play MVP matches.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_transport_ws/**`
- `src/battle_backend/** only for transport/session integration`
- `src/battle_server_app/** only for WS startup config`
- `frontend/telegram_mini_app/**`
- `tests/unit/transport/**`
- `tests/security/**`
- `tests/frontend/**`
- `docs/telegram/**`
- `docs/frontend/**`
- `README.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_core/** unless public snapshot DTO bug blocks integration`
- `src/battle_qt_client/**`
- `deploy/** unless explicitly documenting local dev only`
- `external/**`

## Required quality gates
- Gate A
- Gate B
- Gate E
- Gate F
- Gate G
- Gate J
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Implement WebSocket session lifecycle using the same backend session abstraction.
2. Enforce message size, handshake timeout, ping/pong, idle timeout, and bounded queues.
3. Fix frontend WebSocket state machine so auth is sent after `open`.
4. Send raw Telegram `initData` to backend and never rely on `initDataUnsafe` for authority.
5. Add runtime validation for inbound protocol messages.
6. Render playable arena on Canvas with bases, players, objective, hazards, score, cooldowns, and connection state.
7. Add touch and desktop fallback controls that send intentions only.
8. Add tests for auth failure, malformed messages, frontend validation, and state transitions where practical.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Scenario D is playable locally.
- Frontend strict TypeScript, lint, and build pass.
- Backend validates Telegram auth before accepting Telegram identity.
- Frontend contains no secrets.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Frontend-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
