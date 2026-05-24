# Task 0036: Integrate existing SVG player assets

## State

committed

## Coordinator Preflight

- Branch: `agent/0036-svg-player-assets`.
- Baseline: `master` after local merge of task 0034.
- Task 0035 was checked against existing task 0032 behavior with targeted protocol/WebSocket tests and treated as already satisfied, so no duplicate 0035 implementation branch was created.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0036-integrate-svg-player-assets.md`.
- Allowed implementation scope confirmed: production player assets, Qt client rendering/resource wiring, Web/Telegram canvas rendering, frontend tests, manual checklist/docs, task packet, run note, and CMake Qt resource/Svg wiring only.
- Forbidden scope confirmed: no `battle_core`, backend, protocol, transport, workflow, deploy, gameplay-rule, release-tag, or task 0037 work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0036. Copy an existing safe SVG player asset from the imported snapshot into a canonical production asset path, render it in Qt `ArenaView` and Web/Telegram canvas, preserve team/carrier overlays, rotate local player sprite by available aim/facing direction, add focused tests, and run architecture/SVG safety checks. Do not move gameplay authority into clients and do not start task 0037.

## Implementation-Agent

Assumptions:

- The existing `swordsman.svg` from the imported snapshot is a suitable first player sprite for both teams because team identity is communicated by overlays and labels, not by gameplay-specific SVG content.
- Qt can use `QtSvg` in the local MinGW kit; default non-Qt builds remain unaffected because `BATTLE_BUILD_QT_CLIENT=OFF`.
- Web can serve the repository `assets/` directory through Vite `publicDir`, avoiding a duplicate frontend-local SVG copy.

Changed files:

- `CMakeLists.txt`
- `assets/players/swordsman.svg`
- `docs/agent-runs/0036-integrate-svg-player-assets.md`
- `docs/agent-tasks/0036-integrate-svg-player-assets.md`
- `docs/agent-tasks/README.md`
- `frontend/telegram_mini_app/README.md`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/vite.config.ts`
- `src/battle_qt_client/README.md`
- `src/battle_qt_client/assets.qrc`
- `src/battle_qt_client/ui/ArenaView.cpp`
- `src/battle_qt_client/ui/ArenaView.hpp`
- `src/battle_qt_client/ui/README.md`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `tests/manual/qt_client_checklist.md`

Implementation notes:

- Copied the existing `swordsman.svg` into canonical production path `assets/players/swordsman.svg`.
- Exposed root `assets/` to Vite so Web/Telegram loads `/players/swordsman.svg` without duplicating the asset.
- Added Web canvas image preloading, `drawImage` sprite rendering, local aim rotation, and retained team/carrier/HP overlays.
- Wired touch/keyboard command directions into `ArenaCanvas.setAimDirection` for local sprite facing.
- Embedded the same SVG into Qt through `src/battle_qt_client/assets.qrc` and rendered it via `QSvgRenderer`.
- Preserved Qt team/carrier/HP/local labels and added local aim rotation for the sprite.
- Updated manual/docs to state that player sprites are rendering-only assets.

Checks run:

- `node tests\frontend\telegram_arena_canvas_assets.mjs` - pass.
- `python scripts\ci\validate_architecture_boundaries.py` - pass.
- `git diff --check` - pass with Windows line-ending warnings only.
- `node tests\frontend\telegram_protocol_validation.mjs` - pass.
- `node tests\frontend\telegram_websocket_client_behavior.mjs` - pass.
- `node tests\frontend\telegram_touch_controls_behavior.mjs` - pass.
- `npm.cmd run typecheck` in `frontend/telegram_mini_app` - pass.
- `npm.cmd run lint` in `frontend/telegram_mini_app` - pass.
- `npm.cmd run build` in `frontend/telegram_mini_app` - failed under sandbox while Vite/esbuild loaded config; rerun with escalation - pass.
- `Test-Path frontend\telegram_mini_app\dist\players\swordsman.svg` - true after build.
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass.
- `python scripts\ci\scan_secrets.py` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- Qt configure with `BATTLE_BUILD_QT_CLIENT=ON` and `CMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64` - pass; non-blocking `WrapVulkanHeaders` not found message only.
- Qt build `cmake --build build-qt-mingw --parallel` - pass.
- Qt CTest `ctest --test-dir build-qt-mingw --output-on-failure` - pass.
- `validate_structure.sh` through Git Bash with `python3` shim - pass.

## Test Impact Matrix

Changed behavior:

- Qt and Web/Telegram players render through the existing SVG player asset instead of debug-style filled circles when the asset is available.
- Local Qt and Web sprites rotate by current local aim/movement direction.
- Own/enemy/team labels, HP, and carrier overlays remain client-rendered visual indicators over authoritative snapshots.
- Vite copies the root `assets/` directory into frontend build output.

Tests added/updated:

- Happy path: `telegram_arena_canvas_assets.mjs` verifies both players render through `drawImage` using `/players/swordsman.svg`.
- Corner cases: Web canvas falls back to the existing circle renderer if the image is not loaded yet; Qt falls back if the resource is invalid.
- Invalid input / hostile input: architecture validator checks production SVG assets for `script`, `foreignObject`, and external URL content.
- Authority / ownership: no protocol/backend/core behavior changed; clients still render snapshots and send intentions only.
- Resource bounds / performance: Web caches one `Image`; Qt embeds one resource and keeps one `QSvgRenderer`; no growing runtime collections were added.
- Regression: frontend protocol/WebSocket/touch-control tests still pass; default and Qt CTest suites still pass.
- Manual UI checks: `tests/manual/qt_client_checklist.md` updated to check SVG sprite rendering and fallback-only placeholder circles.

Not tested and why:

- Full manual two-client Qt and browser visual inspection was not executed in this non-interactive session. Automated Web canvas, frontend, Qt build, CTest, and architecture/SVG safety checks passed.

Ready for verification: yes.

## Verification-Agent

Decision: pass.

Verified scope:

- No `battle_core`, backend, protocol, transport, workflow, deploy, release-tag, or task 0037 changes.
- Asset content is copied from the existing imported snapshot and validated by the architecture-boundary SVG checks.
- Untracked roadmap file remains outside the task and unstaged.

Required gates:

- Gate A: pass.
- Gate D: pass.
- Gate E: pass.
- Gate K: pass.
- Gate M: pass.

## Review-Agent

Architecture-Agent:

- Decision: approve.
- Findings: assets remain rendering-only, Qt/Web clients still render snapshots and send intentions only, and no forbidden target dependency is introduced. `QtSvg` is only linked by the Qt client target.
- Required re-checks: architecture-boundary validator and Qt build.

Security-Agent:

- Decision: approve.
- Findings: SVG is checked by the architecture validator; no raw SVG is injected through DOM `innerHTML`; no secrets, auth, parser, or network validation behavior changed.
- Required re-checks: secret scan and architecture-boundary validator.

Frontend-Agent:

- Decision: approve.
- Findings: strict TypeScript, lint, and build pass; canvas preloads a single image and uses `drawImage`; local aim direction is visual-only.
- Required re-checks: frontend typecheck/lint/build and canvas asset test.

Qt-Agent:

- Decision: approve.
- Findings: Qt rendering uses an embedded resource and `QSvgRenderer`; UI/network separation is unchanged; fallback rendering is limited to missing/invalid resource.
- Required re-checks: Qt configure/build/CTest.

## Fix-Agent

Not needed.

## Commit-Agent

- Commit: `frontend: integrate svg player assets`.
- Staged scope matched the task packet.
- Untracked roadmap file was not staged.

## Coordinator Closeout

- Final status: committed.
- Commit hash: recorded by Coordinator final response after commit creation.
- No push, tag, workflow change, deployment file change, gameplay authority change, or task 0037 work was performed.
- Known risk: full manual two-client visual inspection remains pending.
