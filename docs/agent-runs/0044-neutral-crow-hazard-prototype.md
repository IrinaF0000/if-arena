# Task 0044: Neutral crow hazard prototype

## State

commit-ready

## Coordinator Preflight

- Branch: `agent/0044-neutral-crow-hazard-prototype`.
- Baseline: `master` after local merge of task 0043.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0044-neutral-crow-hazard-prototype.md`.
- Allowed implementation scope confirmed: core hazard kind/config, backend hazard serialization, Web/Qt hazard rendering, focused tests/docs.
- Forbidden scope confirmed: no workflows, no `scripts/ci/**`, no deploy files, no protocol version bump, no new player/AI ownership system, no objective carrying by crow, no release tag, no push.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0044. Add a deterministic neutral crow as a server-controlled hazard kind, configured in the default Objective Run arena, serialized in snapshots, and rendered in Web and Qt clients. Keep the crow simple: deterministic seeded center patrol, minor damage/peck radius, no objective ownership, no new player/controller system. Add focused C++ and frontend tests. Do not modify CI workflows, deploy files, protocol versioning, or release state.

## Implementation-Agent

Completed.

Changed files:

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
- `docs/agent-tasks/0044-neutral-crow-hazard-prototype.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0044-neutral-crow-hazard-prototype.md`

Implementation summary:

- Added `HazardKind::Crow` as a deterministic hazard kind, not a player/controller.
- Added a `seed` field to `HazardConfig` for deterministic crow patrol phase.
- Added crow patrol logic around a configured center anchor, with small peck radius/damage and cooldown.
- Added a default center crow to `arena_small_objective_run` tuned not to break the direct capture smoke.
- Serialized `crow` hazards through backend snapshot payloads.
- Updated Telegram Mini App validation/rendering and Qt hazard rendering for a distinct crow marker.

## Test Impact Matrix

Changed behavior:

- Default Objective Run now includes a server-controlled neutral crow hazard near center.
- Crow hazards patrol deterministically from config seed and can peck nearby players for minor damage.
- Snapshot payloads can contain `kind:"crow"` hazards.
- Web and Qt clients have distinct crow rendering paths.

Tests added/updated:

- Happy path: core crow patrol/peck test, backend crow payload assertion, frontend crow parse/render tests.
- Corner cases: deterministic two-engine patrol comparison, default crow tuned to preserve direct Objective Run capture.
- Invalid input / hostile input: frontend hazard kind validation still rejects unknown kinds; existing protocol negative tests remain covered by CTest.
- Authority / ownership: crow is core-owned hazard config/snapshot data; no client or crow can own player state, objective state, or match result.
- Resource bounds / performance: crow adds one fixed default hazard and an O(1) patrol lookup per hazard tick; no queues or unbounded collections changed.
- Regression: full C++ suite, Qt client build, frontend typecheck/lint/build, architecture/agent validators.
- Manual UI checks: no live manual UI session; Qt source was compiled through `BATTLE_BUILD_QT_CLIENT=ON`, and browser canvas rendering was covered by the fake canvas test.

Not tested and why:

- Live browser and live Qt visual inspection were not run. Automated/focused checks covered protocol validation, canvas draw calls, and Qt compilation.

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
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
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

- `npm.cmd run build` used approved escalation because esbuild previously hit sandbox access denial while loading Vite config.
- A stale `build-qt` directory had a different CMake generator, so Qt verification used fresh `build-qt-mingw`.

## Review-Agent

Architecture-Agent: approved. Crow remains hazard config/state in `battle_core`, snapshots remain immutable, and clients still render server state only.

Frontend-Agent: approved. Telegram Mini App validates `crow` hazard kind and draws it without adding authority.

Qt-Agent: approved. Qt hazard renderer has a distinct crow branch and the Qt client target compiled in `build-qt-mingw`.

Security Review Agent: not required. No auth, network parser, Telegram auth, session identity, or deployment behavior changed; secret scan passed.

Performance Review Agent: not required. The patrol is fixed-size per configured hazard and the default map adds one crow.

## Fix-Agent

Completed during implementation.

Fixes:

- Adjusted the old asymmetric hazard test to remove one paired hazard instead of the new self-mirrored crow.
- Tuned the default crow patrol/radius so it adds side pressure without breaking the direct capture regression.

## Commit-Agent

Pending local commit.

## Coordinator Closeout

Ready for local commit and merge. Do not stage the untracked local roadmap file `IF_Arena_Codex_Roadmap_After_Public_0033.md`.
