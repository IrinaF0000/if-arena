# Task 0036: Integrate existing SVG player assets

## Goal

Replace placeholder player circles with an existing SVG player asset in Qt and Web/Telegram rendering while preserving server authority and architecture boundaries.

## Scope

Allowed files:

- `assets/players/**`
- `frontend/telegram_mini_app/**`
- `src/battle_qt_client/**`
- `tests/frontend/**`
- `tests/manual/**`
- `docs/agent-runs/0036-integrate-svg-player-assets.md`
- `docs/agent-tasks/0036-integrate-svg-player-assets.md`
- `docs/agent-tasks/README.md`
- `CMakeLists.txt` only for Qt resource/Svg wiring

Forbidden files:

- `src/battle_core/**`
- `src/battle_backend/**`
- `src/battle_protocol/**`
- `src/battle_transport_tcp/**`
- `src/battle_transport_ws/**`
- `.github/workflows/**`
- `deploy/**`
- gameplay/protocol behavior changes
- release tags
- task 0037 or later

## Required behavior

- Use an existing SVG player asset copied from the imported snapshot into a canonical production asset path.
- Render the SVG player in Qt `ArenaView`.
- Render the SVG player in Web/Telegram canvas.
- Preserve existing own/enemy overlays: own player is blue/cyan, enemy is red/orange, carrier is gold.
- Rotate local player sprite by current aim/facing direction where available.
- Do not inject raw SVG through `innerHTML`.
- Do not define gameplay via assets.
- Validate SVG assets contain no `script`, `foreignObject`, or external URLs.

## Required quality gates

- Gate A. Every implementation task
- Gate D. Qt client
- Gate E. Telegram Mini App
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_touch_controls_behavior.mjs`
- `cd frontend/telegram_mini_app && npm.cmd run typecheck`
- `cd frontend/telegram_mini_app && npm.cmd run lint`
- `cd frontend/telegram_mini_app && npm.cmd run build`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- Qt build/CTest when Qt is available
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes for SVG asset safety
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Qt and Web players render through the SVG asset with team/carrier overlays.
- No authoritative gameplay logic moves into clients.
- Architecture-boundary validator passes.
- SVG asset is production-safe.
- Required checks pass or documented environment blockers exist.

## Rollback note

One scoped task commit. Revert the asset, Qt resource/rendering changes, Web canvas rendering changes, tests, and docs for rollback.
