# Task 0043: Map contest pressure and neutral hazards

## State

commit-ready

## Coordinator Preflight

- Branch: `agent/0043-map-contest-pressure-neutral-hazards`.
- Baseline: `master` after local merge of task 0042.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0043-map-contest-pressure-and-neutral-hazards.md`.
- Allowed implementation scope confirmed: default Objective Run arena config, authoritative obstacle snapshots, Telegram Mini App obstacle parsing/rendering, focused tests/docs.
- Forbidden scope confirmed: no workflows, no `scripts/ci/**`, no deploy files, no protocol version bump, no crow AI, no release tag, no push.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0043. Improve default Objective Run center contest pressure in the existing scenario/config layer, expose server-authoritative obstacle cells in snapshots, and render those obstacle cells in the Telegram Mini App. Keep dimensions, symmetry, central objective, and direct playable routes. Add focused deterministic C++ tests and frontend validation/render tests. Do not modify CI workflows, deploy files, protocol versioning, or crow behavior.

## Implementation-Agent

Completed.

Changed files:

- `src/battle_core/ArenaConfig.hpp`
- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/README.md`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/README.md`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/game/ArenaCanvas.ts`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/frontend/telegram_protocol_validation.mjs`
- `tests/frontend/telegram_arena_canvas_assets.mjs`
- `tests/frontend/telegram_websocket_client_behavior.mjs`
- `docs/game/MAP_AND_FAIRNESS.md`
- `docs/agent-tasks/0043-map-contest-pressure-and-neutral-hazards.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0043-map-contest-pressure-and-neutral-hazards.md`

Implementation summary:

- Added symmetric center wall shoulders to the default 21x13 Objective Run arena while preserving the central lane and mirrored side approaches.
- Added hazard validation for duplicate hazard cells and base-zone overlap.
- Added authoritative obstacle cells to `BattleSnapshot` and backend snapshot payloads.
- Updated the Telegram Mini App snapshot schema and canvas renderer to validate and draw authoritative obstacle blockers.
- Updated core/backend/game docs for obstacle snapshot behavior.

## Test Impact Matrix

Changed behavior:

- Default Objective Run has clearer center obstacle pressure and three tested approach gates to the objective.
- Authoritative snapshots now include obstacle cells.
- Browser snapshots must include `obstacles`, and the Telegram Mini App renders them as blockers.
- Arena validation rejects duplicate hazard cells and hazards inside base zones.

Tests added/updated:

- Happy path: canonical arena validation, three objective routes, backend snapshot payload, Telegram snapshot parsing, canvas obstacle rendering.
- Corner cases: mirrored route gates, duplicate hazard cells, hazards overlapping base zones.
- Invalid input / hostile input: Telegram protocol validation rejects invalid obstacle coordinates.
- Authority / ownership: backend broadcasts obstacles from core snapshots; frontend only renders server-provided obstacle cells.
- Resource bounds / performance: obstacle list is fixed-size scenario data in snapshots; no queues, workers, or unbounded collections changed.
- Regression: full C++ test suite, frontend protocol/canvas/client tests, typecheck, lint, build, architecture and agent validators.
- Manual UI checks: not run as a live browser session; canvas rendering is covered by the fake canvas test.

Not tested and why:

- Live browser visual inspection was not completed. `file://` navigation is blocked by the Browser tool, and the follow-up localhost navigation attempt was rejected because the tool call was malformed. The dev server process was cleaned up and no live UI check is claimed.
- Qt obstacle rendering was not changed because the default build does not compile Qt UI sources; this task keeps the verified client render path to the Telegram Mini App.

Ready for verification: yes.

## Verification-Agent

Passed.

Checks run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_protocol_validation.mjs`
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

- Direct `npm run ...` through PowerShell failed because `npm.ps1` execution is blocked by system policy. The same checks passed through `npm.cmd`.
- First `npm.cmd run build` attempt inside the sandbox failed when esbuild hit an access-denied read while loading `vite.config.ts`; rerunning the same build with approved escalation passed.

## Review-Agent

Architecture-Agent: approved. Obstacles remain core-owned scenario data and are exposed through immutable snapshots; clients still send intentions only.

Frontend-Agent: approved. Telegram Mini App validates obstacle payloads and renders only server-provided blockers.

Security Review Agent: not required. No auth, network parser, session identity, Telegram auth, or deployment behavior changed; secret scan passed.

Performance Review Agent: not required. Runtime change is a small fixed obstacle vector in snapshots for the canonical 21x13 scenario.

## Fix-Agent

Not required.

## Commit-Agent

Pending local commit.

## Coordinator Closeout

Ready for local commit and merge. Do not stage the untracked local roadmap file `IF_Arena_Codex_Roadmap_After_Public_0033.md`.
