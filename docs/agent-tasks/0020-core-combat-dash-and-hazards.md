# Task 0020: Add combat, dash, and MVP hazards

## Task title
`Add combat, dash, and MVP hazards`

## Phase
`Phase 1/arena gameplay`

## Goal
Add MVP attack, cooldown, dash/dodge, and deterministic neutral hazards needed for a tactically playable Objective Run match.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_core/**`
- `tests/unit/core/**`
- `config/scenarios/**`
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
- Gate G
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add attack intention with direction/aim validation.
2. Add attack cooldown and hit detection owned by server.
3. Add dash intention, cooldown, and collision-safe movement.
4. Add mine and tower hazard models with deterministic timing and telegraph/event output.
5. Ensure successful hit on carrier triggers objective drop.
6. Add tests for cooldowns, invalid attacks, dash bounds, hazard hits, and drop-on-hit.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Client cannot decide hit, cooldown, damage, or drop.
- Hazards are deterministic for fixed ticks.
- Combat supports MVP gameplay without adding complex classes.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
