# Task 0066: Mobile layout alignment

## Goal

Align the Telegram Mini App layout with the desktop semantics while keeping mobile arena focus, compact score/status, bottom controls, and a collapsible match panel.

## Scope

Allowed files:

- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/src/style.css`
- `tests/frontend/**`
- `docs/agent-runs/0066-mobile-layout-alignment.md`
- `docs/agent-tasks/0066-mobile-layout-alignment.md`
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

- Top mobile status line shows compact score/objective/scenario information from authoritative snapshots.
- Connection/create/join/next-match controls live in a collapsible match panel.
- Arena remains central and bottom controls remain reachable.
- Mobile object visuals continue using the same snapshot semantics and SVG assets as desktop.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- TypeScript/frontend quality rules

## Required checks

- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `python tests/integration/mobile/rematch_same_screen_mobile.py`
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
- Verification-Agent: yes

## Acceptance criteria

- Mobile first screen keeps score/status compact and arena-focused.
- Service controls are available without covering the arena.
- Same-screen next-match control remains available after match over.
- Existing mobile protocol/client/scenario checks pass.

## Rollback note

Revert this task commit to return the Mini App to the previous fixed toolbar layout.
