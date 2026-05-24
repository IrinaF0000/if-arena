# Task 0045: Visual combat and objective readability polish

## Goal

Make combat, carrying, pickup, drop, capture, cooldowns, and match end readable without reading logs.

## Scope

Allowed files:

- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `frontend/telegram_mini_app/src/main.ts`
- `src/battle_qt_client/ui/ArenaView.cpp`
- `src/battle_qt_client/ui/ArenaView.hpp`
- `src/battle_qt_client/ui/README.md`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `docs/agent-runs/0045-visual-combat-and-objective-readability-polish.md`
- `docs/agent-tasks/0045-visual-combat-and-objective-readability-polish.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- authoritative gameplay rules
- protocol version changes
- generated frontend `dist/**`
- release tags
- pushing to GitHub

## Required behavior

- Add clearer browser combat/objective feedback using existing snapshots and event batches.
- Show attack direction/range, dash trail, hit flash, pickup/drop/capture feedback, match-over overlay, and cooldown feedback.
- Keep all visuals presentation-only; clients must not infer or mutate authoritative game state.
- Add a small Qt readability polish path that compiles with `BATTLE_BUILD_QT_CLIENT=ON`.

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
- `cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON`
- `cmake --build build-qt-mingw --parallel`
- `ctest --test-dir build-qt-mingw --output-on-failure`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
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

- Browser canvas draw-call tests cover combat/objective readability elements.
- WebSocket client behavior test proves event batches reach the presentation path without automatic authority commands.
- Qt client target compiles with the readability overlay changes.
- No gameplay, protocol version, CI, deploy, or generated dist changes are committed.

## Rollback note

Revert this task commit to remove the visual readability presentation layer.
