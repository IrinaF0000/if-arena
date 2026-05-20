# Task 0021: Add playable gameplay protocol messages

## Task title
`Add playable gameplay protocol messages`

## Phase
`Phase 2 - protocol foundation`

## Goal
Extend the transport-independent protocol with typed messages for handshake, auth result, create/join match, input intentions, snapshots, event batches, ping/pong, and errors.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_protocol/**`
- `tests/unit/protocol/**`
- `docs/architecture/PROTOCOL.md`
- `docs/project/PRODUCT_REQUIREMENTS.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_core/** except DTO discussion docs`
- `src/battle_transport_*/*`
- `src/battle_qt_client/**`
- `frontend/**`
- `external/**`

## Required quality gates
- Gate A
- Gate B
- Gate C
- Gate F
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Define versioned message types needed by playable MVP.
2. Represent client input as intentions only: movement, aim, attack, dash, interact.
3. Represent server snapshots/events without exposing authority to clients.
4. Validate required fields, enum values, string lengths, numeric ranges, sequence numbers, and message order where applicable.
5. Add negative tests for malformed, unknown, oversized, and out-of-order messages.
6. Update protocol docs.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Protocol remains transport-independent.
- All client authority fields are rejected or ignored safely.
- Parser errors do not cross transport boundary unhandled.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: no
- Verification-Agent: yes

## Rollback note
One scoped commit.
