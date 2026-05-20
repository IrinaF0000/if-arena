# Task: Create WebSocket transport skeleton

## Phase

Phase 7. WebSocket gateway

## Goal

Create WebSocket transport module boundaries and session adapter skeleton.

## Allowed files

- `src/battle_transport_ws/`
- `docs/architecture/TRANSPORT_ABSTRACTION.md`
- `docs/telegram/TELEGRAM_SECURITY.md`
- `tests/unit/transport/`

## Forbidden files

- `frontend/telegram_mini_app/` except protocol notes
- `src/battle_core/`

## Required quality gates

- Gate A
- Gate B
- Gate F
- Gate G

## Security impact

high

## Performance/scalability impact

high

## Architecture impact

high

## Token budget instructions

- Read WebSocket docs, transport abstraction, battle_transport_ws only.

## Implementation steps

1. Add WebSocket session abstraction.
2. Add message size limit constants.
3. Add backend session adapter skeleton.
4. Add tests for oversized/malformed message handling where possible.
5. Document chosen WebSocket library if selected.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- Oversized WebSocket message rejected.
- Malformed message does not crash.
- No dependency from core to WebSocket.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope remains narrow.
- Checks pass or blockers are documented.
- Run note is updated.
