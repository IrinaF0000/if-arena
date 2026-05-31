# Task 0077: Corrective arena overlay cleanup

## Goal

Move long HUD, status, and hazard legend text out of the desktop and Mini App arena field.

## Scope

Allowed files:

- `CMakeLists.txt`
- `src/battle_qt_client/ui/**`
- `src/battle_qt_client/tests/**`
- `frontend/telegram_mini_app/src/**`
- `tests/frontend/**`
- `docs/agent-runs/0077-corrective-arena-overlay-cleanup.md`
- `docs/agent-tasks/0077-corrective-arena-overlay-cleanup.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay authority changes
- protocol/schema changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Desktop arena no longer paints hazard legend text over the field.
- Desktop side panel keeps connection, status, match, log, and hazard information outside the arena.
- Mini App canvas no longer paints long HUD/status or hazard legend text over the arena after snapshots.
- Mini App keeps score/status in the topbar and hazard information in a compact drawer outside the canvas.
- Short gameplay banners and match-over overlay may remain in the arena.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Qt quality rules
- TypeScript/frontend quality rules
- Testing rules

## Required checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=<local Qt path>`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure -R battle_qt_client`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `node tests/frontend/telegram_main_layout_contract.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: no
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Hazard legend text is rendered outside the arena field in both clients.
- Canvas tests fail if old HUD or hazard legend text returns to active arena rendering.
- Qt tests cover the side-panel hazard text source.
- Required checks pass or any skipped check is explicitly justified.

## Rollback note

Revert this task commit to restore the previous in-arena HUD and hazard legend behavior.
