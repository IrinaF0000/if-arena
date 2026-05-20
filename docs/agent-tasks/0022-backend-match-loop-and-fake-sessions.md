# Task 0022: Add backend match loop and fake sessions

## Task title
`Add backend match loop and fake sessions`

## Phase
`Phase 3 - backend application layer`

## Goal
Implement the authoritative backend match manager, in-process match worker, fixed tick loop, command queue, snapshots/events, and fake sessions for integration tests.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_backend/**`
- `src/battle_core/** only for required public facade changes`
- `src/battle_protocol/** only for required DTO integration`
- `tests/unit/backend/**`
- `tests/integration/**`
- `docs/architecture/**`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_transport_*/*`
- `src/battle_qt_client/**`
- `frontend/**`
- `deploy/**`
- `external/**`

## Required quality gates
- Gate A
- Gate C
- Gate F
- Gate G
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add match ids, player ids, session ownership, and join code flow.
2. Add fake session adapter for tests.
3. Add bounded command queue and outgoing snapshot/event queue.
4. Add fixed tick loop or deterministic tick driver for tests.
5. Validate command ownership, match state, sequence, and rate limits.
6. Broadcast snapshots/events to matched sessions.
7. Add in-process integration test for Scenario A.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Backend can run a complete match without real sockets.
- Slow or spammy fake session cannot grow memory without bound.
- Command from wrong session/player is rejected.
- Metrics counters exist for commands, rejections, snapshots, and disconnects.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
