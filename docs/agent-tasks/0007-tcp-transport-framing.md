# Task: Implement TCP framing

## Phase

Phase 4. Raw TCP transport

## Goal

Implement length-prefixed TCP frame codec with tests before full socket integration.

## Allowed files

- `src/battle_transport_tcp/`
- `tests/unit/transport/`
- `docs/architecture/PROTOCOL.md`

## Forbidden files

- `src/battle_core/`
- `frontend/`

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

medium

## Token budget instructions

- Read protocol doc and transport_tcp files only.

## Implementation steps

1. Implement standalone frame decoder/encoder.
2. Support partial frames.
3. Support multiple frames in one buffer.
4. Reject oversized frame length before allocation.
5. Return explicit errors.
6. Add tests.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- Partial frame test.
- Combined frames test.
- Oversized length test.
- Invalid length/error state test.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope remains narrow.
- Checks pass or blockers are documented.
- Run note is updated.
