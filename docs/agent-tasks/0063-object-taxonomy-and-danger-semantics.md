# Task 0063: Object taxonomy and danger semantics

## Goal

Expose authoritative object semantics for blockers, hazards, and neutral bots so desktop and mobile clients can render readable danger markers without inferring gameplay meaning from hardcoded colors or shapes.

## Scope

Allowed files:

- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_qt_client/game/**`
- `src/battle_qt_client/ui/ArenaView.*`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/frontend/**`
- `tests/integration/gameplay/scenario_runner.py`
- `docs/architecture/PROTOCOL.md`
- `docs/architecture/TECHNICAL_ARCHITECTURE.md`
- `config/scenarios/README.md`
- `docs/agent-runs/0063-object-taxonomy-and-danger-semantics.md`
- `docs/agent-tasks/0063-object-taxonomy-and-danger-semantics.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay rule changes in `battle_core`
- scenario route/config redesign
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Snapshot obstacles expose stable object metadata: id, kind, visual id, position, movement blocking, damage/drop semantics, range radius, cooldown, and neutral ownership.
- Snapshot hazards expose visual id, movement blocking, drop semantics, range radius, and neutral ownership beside existing authoritative hazard metadata.
- Desktop and mobile clients validate the new fields before rendering.
- Desktop and mobile render compact damage/drop markers and readable blockers using server-provided metadata.
- Documentation defines the object taxonomy without introducing a new authoritative client model.

## Required quality gates

- Gate A. Every implementation task
- Gate B. Protocol/input validation
- Gate C. Backend/gameplay authority
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity

## Required checks

- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `python tests/integration/desktop/rematch_same_screen_desktop.py`
- `python tests/integration/mobile/rematch_same_screen_mobile.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Clients no longer need to infer damage/drop/block semantics from visual colors or hazard kind alone.
- Blocking obstacles are visibly distinct in both clients.
- Damage and drop hazards show compact secondary markers and subtle range circles.
- Protocol and scenario tests reject missing or malformed object semantic metadata.
- `battle_core` remains isolated and authoritative gameplay rules are unchanged.

## Rollback note

Revert this task commit to return snapshots and client rendering to the prior hazard/obstacle metadata model.
