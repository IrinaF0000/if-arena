# Task 0044: Neutral crow hazard prototype

## Goal

Add a simple server-controlled neutral crow hazard that creates center pressure without becoming a third player.

## Scope

Allowed files:

- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/ArenaConfig.hpp`
- `src/battle_core/README.md`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/README.md`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `src/battle_qt_client/ui/ArenaView.cpp`
- `src/battle_qt_client/ui/README.md`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/frontend/telegram_protocol_validation.mjs`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `docs/game/ARENA_OBJECTS.md`
- `docs/agent-runs/0044-neutral-crow-hazard-prototype.md`
- `docs/agent-tasks/0044-neutral-crow-hazard-prototype.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- protocol version changes
- new AI/player ownership systems
- crow objective carrying
- release tags
- pushing to GitHub

## Required behavior

- Add `crow` as a deterministic neutral hazard kind, not as a player/entity controller.
- Crow patrols near the center from scenario config using a deterministic seed.
- Crow pecks nearby players for minor damage with cooldown.
- Crow never carries or owns the objective.
- Crow appears in authoritative snapshots and is visible in Web and Qt clients.
- Keep damage and range small enough that the hazard adds pressure without dominating the match.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement
- Frontend checks for Telegram Mini App changes
- Qt-Agent review for Qt rendering changes

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm.cmd run typecheck` from `frontend/telegram_mini_app`
- `npm.cmd run lint` from `frontend/telegram_mini_app`
- `npm.cmd run build` from `frontend/telegram_mini_app`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Frontend-Agent: yes
- Qt-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Core tests prove deterministic crow patrol and peck damage.
- Default arena exposes a crow hazard near the objective.
- Backend snapshot payload serializes `kind:"crow"`.
- Telegram Mini App validates and renders crow hazards.
- Qt hazard renderer has a distinct crow rendering path.
- No workflow, deploy, protocol version, or player-authority changes are made.

## Rollback note

Revert this task commit to remove the crow hazard kind, default arena crow config, and client rendering support.
