# Task 0018: Add player movement and collision

## Task title
`Add player movement and collision`

## Phase
`Phase 1/arena gameplay`

## Goal
Implement fixed-step smooth movement over the logical arena with canonical coordinates, obstacle collision, arena bounds, spawn positions, and base zones.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_core/**`
- `tests/unit/core/**`
- `docs/game/**`
- `docs/architecture/**`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_transport_*/*`
- `src/battle_backend/** unless only DTO boundary is explicitly needed`
- `src/battle_qt_client/**`
- `frontend/**`
- `external/**`

## Required quality gates
- Gate A
- Gate C
- Gate G if movement hot paths or transforms are performance-sensitive
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add canonical coordinate types and movement vectors.
2. Add player state with position, velocity or desired movement, team, spawn, and base association.
3. Apply server-side movement commands in fixed ticks.
4. Clamp or reject movement through bounds and obstacles.
5. Add local-view transform helpers only if they stay transport/UI independent.
6. Add deterministic tests for movement, collision, spawn, base entry, and coordinate transform.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Players cannot leave arena or pass through obstacles.
- Movement is deterministic for a fixed sequence of inputs and ticks.
- Clients still cannot set position directly.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
