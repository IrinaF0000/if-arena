# Task: Implement backend session model

## Phase

Phase 3. Backend application layer

## Goal

Create transport-agnostic backend sessions, session registry, and fake-session tests.

## Allowed files

- `src/battle_backend/`
- `tests/unit/backend/`
- `docs/architecture/TRANSPORT_ABSTRACTION.md`

## Forbidden files

- `src/battle_transport_tcp/`
- `src/battle_transport_ws/`
- `src/battle_qt_client/`
- `frontend/`

## Required quality gates

- Gate A
- Gate C
- Gate F
- Gate G

## Security impact

high

## Performance/scalability impact

high

## Architecture impact

high

## Token budget instructions

- Read battle_backend, protocol types, transport abstraction doc.
- Do not inspect UI clients.

## Implementation steps

1. Add SessionId/ConnectionId/PlayerId types.
2. Add abstract outbound session interface.
3. Add session registry.
4. Add handshake/auth state enum.
5. Add bounded outgoing queue placeholder policy.
6. Add fake session tests.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- Session can be created/closed.
- Command before auth rejected.
- Outgoing queue bound test.
- Metrics counters placeholder tested if implemented.

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
