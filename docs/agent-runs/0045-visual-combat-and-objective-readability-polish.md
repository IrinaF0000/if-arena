# Task 0045: Visual combat and objective readability polish

## State

commit-ready

## Coordinator Preflight

- Branch: `agent/0045-visual-combat-objective-readability`.
- Baseline: `master` after local merge of task 0044.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0045-visual-combat-and-objective-readability-polish.md`.
- Allowed implementation scope confirmed: Telegram Mini App presentation, Qt presentation, focused tests/docs.
- Forbidden scope confirmed: no authoritative gameplay changes, no protocol version changes, no workflows, no `scripts/ci/**`, no deploy files, no generated `dist/**`, no release tag, no push.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0045. Add presentation-only visual combat/objective readability to the Telegram Mini App using existing snapshots and event batches, plus a small Qt readability overlay that compiles. Cover browser draw calls and WebSocket event flow with focused tests. Do not change gameplay authority, protocol version, CI/deploy files, or generated frontend dist.

## Implementation-Agent

Completed.

Changed files:

- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `frontend/telegram_mini_app/src/main.ts`
- `src/battle_qt_client/ui/ArenaView.cpp`
- `src/battle_qt_client/ui/ArenaView.hpp`
- `src/battle_qt_client/ui/README.md`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `docs/agent-tasks/0045-visual-combat-and-objective-readability-polish.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0045-visual-combat-and-objective-readability-polish.md`

Implementation summary:

- Added Telegram Mini App presentation-only action feedback for attack and dash commands.
- Added browser canvas attack range/direction indicator, dash/attack trail, hit flash labels, pickup/drop/capture event labels, local cooldown bars, and match-over overlay.
- Routed existing `event_batch` payloads into the canvas presentation layer without changing protocol or server authority.
- Added Qt objective lock/captured glow and match-over overlay; verified with a Qt-enabled build.

## Test Impact Matrix

Changed behavior:

- Browser client visuals now make attack direction/range, dash/attack actions, hit/capture events, local cooldowns, and match end readable on the canvas.
- Qt arena view shows objective lock/captured state more clearly and overlays match-over score.
- No authoritative gameplay, protocol version, transport, or backend behavior changed.

Tests added/updated:

- Happy path: canvas draw-call test covers player sprites, carrier overlay, obstacles, crow, range indicator, event labels, cooldown bars, and match-over overlay.
- Corner cases: WebSocket behavior test confirms event batches are delivered as presentation events without creating extra input commands.
- Invalid input / hostile input: no parser boundary changed; existing frontend protocol/runtime validation remains in place.
- Authority / ownership: visuals use snapshots/events only and do not mutate HP, score, objective, cooldowns, or match result.
- Resource bounds / performance: feedback arrays are bounded by the latest event batch; no queues or timers added.
- Regression: C++ suite, Qt build/test, frontend typecheck/lint/build, agent/architecture/secret validators.
- Manual UI checks: no live manual UI session; browser draw-call tests and Qt compilation verify the touched rendering paths.

Not tested and why:

- Live browser/Qt visual inspection was not run. The task is covered by deterministic draw-call tests and a Qt-enabled build.

Ready for verification: yes.

## Verification-Agent

Passed.

Checks run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON`
- `cmake --build build-qt-mingw --parallel`
- `ctest --test-dir build-qt-mingw --output-on-failure`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`
- `bash scripts/ci/validate_structure.sh` with the local Windows `python3` shim

Notes:

- `npm.cmd run build` used approved escalation because Vite/esbuild previously required it in this workspace.
- No generated `dist/**` files are staged or changed.

## Review-Agent

Architecture-Agent: approved. Changes are presentation-only and consume existing snapshots/events.

Frontend-Agent: approved. Browser UI keeps runtime validation boundaries and adds no state authority.

Qt-Agent: approved. Qt rendering changes compiled in `build-qt-mingw`.

Security Review Agent: not required. No auth, parser schema, transport, Telegram auth, session identity, or deployment behavior changed; secret scan passed.

Performance Review Agent: not required. Rendering additions are fixed per snapshot/event batch and do not alter server loops or queues.

## Fix-Agent

Completed during implementation.

Fixes:

- Adjusted the canvas test after multiple render passes created more than two sprite draw calls.
- Fixed TypeScript exact optional property handling for event feedback objects.
- Fixed a lint `prefer-const` finding.

## Commit-Agent

Pending local commit.

## Coordinator Closeout

Ready for local commit and merge. Do not stage the untracked local roadmap file `IF_Arena_Codex_Roadmap_After_Public_0033.md`.
