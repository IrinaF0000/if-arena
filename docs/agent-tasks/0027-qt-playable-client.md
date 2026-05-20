# Task 0027: Complete Qt playable client

## Task title
`Complete Qt playable client`

## Phase
`Phase 6 - Qt desktop client`

## Goal
Implement the Qt desktop client over raw TCP so it can play the same MVP match as CLI and Telegram clients.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_qt_client/**`
- `CMakeLists.txt`
- `tests/manual/qt_client_checklist.md`
- `docs/game/CONTROLS_AND_UI.md`
- `docs/game/PLAYER_ORIENTED_VIEW.md`
- `README.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_core/**`
- `src/battle_backend/** unless protocol integration bug blocks client work`
- `src/battle_transport_ws/**`
- `frontend/**`
- `deploy/**`
- `external/**`

## Required quality gates
- Gate A
- Gate D
- Gate C for client authority checks
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Set up real Qt target and dependency discovery.
2. Implement network client separate from widgets.
3. Add connection, lobby/create/join, arena rendering, HUD, event log, and error states.
4. Add keyboard/mouse controls for move, aim, attack, dash, interact.
5. Implement player-oriented rendering and input transform.
6. Ensure no blocking network operations on UI thread.
7. Update manual checklist with concrete steps and expected visuals.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Scenario E is playable locally.
- Qt client does not link to battle_core.
- UI thread remains responsive during connect, play, and disconnect.
- Manual checklist is updated and passed.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: no unless auth/network parsing changes
- Performance-Agent: no unless render loop/queues change
- Qt-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
