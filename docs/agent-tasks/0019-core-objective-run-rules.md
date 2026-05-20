# Task 0019: Add Objective Run rules

## Task title
`Add Objective Run rules`

## Phase
`Phase 1/arena gameplay`

## Goal
Implement the central Objective Run lifecycle: pickup, carrier slowdown, drop, pickup lock, capture, respawn, score, and win condition.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_core/**`
- `tests/unit/core/**`
- `docs/game/**`
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
- Gate G if tick-loop state changes are touched
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add objective state: at center, dropped, carried, respawning, captured.
2. Add interact command validation for pickup.
3. Apply carrier speed multiplier.
4. Add drop state and pickup lock timer.
5. Add capture validation when carrier enters own base.
6. Add score tracking and configured win condition.
7. Emit core events for pickup, drop, capture, score, and match end.
8. Add deterministic tests for the full Scenario A path.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Scenario A from the playable game plan passes as a deterministic core test.
- Immediate re-pickup during pickup lock fails.
- Capture at enemy base fails.
- Server-side command validation owns all decisions.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes for command validation
- Performance-Agent: yes if tick loop changes
- Verification-Agent: yes

## Rollback note
One scoped commit.
