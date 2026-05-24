# Task 0043: Map contest pressure and neutral hazards

## Goal

Make the default Objective Run map more genuinely contested while keeping obstacles and hazards server-authoritative.

## Scope

Allowed files:

- `src/battle_core/ArenaConfig.hpp`
- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/README.md`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/README.md`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/frontend/telegram_protocol_validation.mjs`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `docs/game/MAP_AND_FAIRNESS.md`
- `docs/agent-runs/0043-map-contest-pressure-and-neutral-hazards.md`
- `docs/agent-tasks/0043-map-contest-pressure-and-neutral-hazards.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- protocol version changes
- new project names
- complex AI or crow behavior
- release tags
- pushing to GitHub

## Required behavior

- Improve the default `arena_small_objective_run` layout with clearer center obstacle pressure.
- Preserve 21x13 dimensions, 180-degree rotational symmetry, central objective spawn, and symmetric teams.
- Keep obstacles and hazards server-authoritative.
- Expose authoritative obstacle cells in snapshots so browser clients do not hit invisible blockers.
- Render authoritative obstacle cells in the Telegram Mini App arena.
- Keep crow/neutral AI hazards for a later task.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement
- Frontend checks for Telegram Mini App changes

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `npm run typecheck` from `frontend/telegram_mini_app`
- `npm run lint` from `frontend/telegram_mini_app`
- `npm run build` from `frontend/telegram_mini_app`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Frontend-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- The default arena has two to three clear obstacle-shaped routes to the center objective.
- Center obstacles and hazards are symmetric and do not overlap spawns, bases, objective spawn, or each other.
- Browser snapshots validate and render authoritative obstacle cells.
- Existing direct movement and capture tests remain passing.
- No CI workflow, deploy, release, or protocol version changes are made.

## Rollback note

Revert this task commit to restore the prior default map layout and snapshot obstacle payload handling.
