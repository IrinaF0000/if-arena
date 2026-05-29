# Task 0064: SVG hazard visual refresh

## Goal

Use shared project-owned SVG assets for blockers and neutral hazards so desktop and mobile render bots/hazards with the same visual IDs and clearer silhouettes.

## Scope

Allowed files:

- `assets/svg/*.svg`
- `src/battle_qt_client/assets.qrc`
- `src/battle_qt_client/ui/ArenaView.*`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `docs/agent-runs/0064-svg-hazard-visual-refresh.md`
- `docs/agent-tasks/0064-svg-hazard-visual-refresh.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay rules
- protocol/schema changes
- scenario config changes
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- `visualId` values render through shared SVG assets in both Qt and Telegram Mini App.
- Add a blocking obstacle SVG asset matching the hazard/player style.
- Make mine, tower, and crow silhouettes larger and clearer without changing gameplay radius/range/collision.
- Keep compact damage/drop markers and range circles from task 0063.
- Preserve fallback drawing if an SVG is unavailable.

## Required quality gates

- Gate A. Every implementation task
- Gate D. Qt client
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity

## Required checks

- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- Browser smoke of built Mini App
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Desktop and mobile use the same `visualId` names for obstacle, mine, tower, and crow assets.
- Hazard/bot bodies are rendered as readable SVGs with range circles and secondary danger markers still visible.
- Existing paired gameplay scenarios and frontend asset tests pass.

## Rollback note

Revert this task commit to return hazards/blockers to code-drawn primitives.
