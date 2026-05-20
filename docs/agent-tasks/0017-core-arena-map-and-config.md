# Task 0017: Add arena map and config model

## Task title
`Add arena map and config model`

## Phase
`Phase 1/arena gameplay`

## Goal
Add a battle_core arena configuration model for the 21x13 Objective Run map, including bases, spawns, obstacles, objective spawn, and symmetric map validation.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_core/**`
- `tests/unit/core/**`
- `config/scenarios/arena_small_objective_run.json`
- `docs/game/**`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_transport_*/*`
- `src/battle_qt_client/**`
- `frontend/**`
- `.github/workflows/**`
- `external/**`

## Required quality gates
- Gate A
- Gate C
- Gate F if parsing untrusted config is touched
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Define core data structures for arena dimensions, cells, obstacles, bases, spawns, and objective spawn.
2. Add config loading or a minimal internal config DTO, depending on existing parser boundaries.
3. Validate size, bounds, required objects, and 180-degree rotational symmetry.
4. Add tests for valid map, asymmetric map, out-of-bounds objects, missing bases, and invalid objective spawn.
5. Update gameplay docs if assumptions change.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Valid 21x13 scenario is accepted.
- Asymmetric or malformed maps are rejected with safe errors.
- battle_core remains independent of transport, UI, Telegram, and process server code.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes if parser/config code changes
- Performance-Agent: no
- Verification-Agent: yes

## Rollback note
One scoped commit.
