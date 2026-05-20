# Task 0018: Core Player Movement And Collision

State: `implementing`

Branch: `agent/0018-core-player-movement-and-collision`

## Coordinator Preflight

Task packet: `docs/agent-tasks/0018-core-player-movement-and-collision.md`.

Sequential order:

- Previous task `0017-core-arena-map-and-config` is terminal and committed on its task branch.
- Current task branch was created from that sequential base.

Allowed files:

- `src/battle_core/**`
- `tests/unit/core/**`
- `docs/game/**`
- `docs/architecture/**`
- `docs/agent-runs/`

Forbidden files:

- `src/battle_transport_*/*`
- `src/battle_backend/**` unless only DTO boundary is explicitly needed
- `src/battle_qt_client/**`
- `frontend/**`
- `external/**`

Required gates and reviews:

- Gate A: architecture boundaries.
- Gate C: deterministic gameplay tests.
- Gate G: movement/collision path review.
- Gate K: sequential pipeline.
- Gate L: playable scenario integrity.
- Architecture-Agent: required.
- Performance-Agent: required.
- Verification-Agent: required.

Implementation-Agent prompt:

```text
Implement task 0018 only. Extend battle_core with canonical movement vectors, smooth world position, desired movement, team/spawn/base state, fixed-step movement, arena bounds collision, obstacle collision, base entry detection, and transport/UI-independent player-view coordinate helpers. Add deterministic tests for movement, collision, spawn/base association, coordinate transforms, and server authority. Do not edit forbidden paths and do not commit.
```

## Implementation-Agent Output

State transition: `ready -> implementing`.

Assumptions:

- Movement remains server-authoritative in `battle_core`; clients still submit intentions only.
- `Vec2i position` remains in snapshots for current callers, while `Vec2d worldPosition` adds smooth canonical movement state.
- Player-view helpers are pure coordinate transforms and do not introduce UI/client dependencies.

Changed files:

- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/ArenaConfig.hpp`
- `src/battle_core/README.md`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `docs/agent-runs/0018-core-player-movement-and-collision.md`

Implemented:

- Added `Vec2d`, `MovementVector`, `ArenaTeam`, and `BaseZoneConfig`.
- Extended match/player snapshots with team, spawn, smooth world position, desired movement, and own-base state.
- Added fixed-step desired movement: move commands set direction, stop clears it, ticks apply deterministic movement.
- Added arena bounds checks and swept obstacle collision so players cannot move through blocked cells, including high-speed ticks.
- Added base entry detection using server-owned player team and base zones.
- Added player-oriented coordinate/input transform helpers for red/blue views while preserving canonical server coordinates.
- Updated battle_core README with the movement/collision boundary.

Checks run during implementation:

- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.

Test Impact Matrix:

Changed behavior:

- `BattleEngine` now stores and advances smooth canonical world positions with desired movement.
- Movement is blocked by arena bounds and obstacle sweep checks.
- Snapshots expose player team, spawn, desired movement, and own-base state.
- `ArenaConfig.hpp` exposes pure player-view coordinate/input transform helpers.

Tests added/updated:

- Happy path: smooth movement advances by configured speed and canonical arena spawns populate match snapshots.
- Corner cases: stop command halts persistent movement; non-canonical arena dimensions from task 0017 remain rejected.
- Invalid input / hostile input: movement outside bounds and movement through obstacles are blocked by the server.
- Authority / ownership: existing tests still reject unknown-player commands and unsupported client-owned authority commands; clients still cannot set position directly.
- Resource bounds / performance: obstacle collision uses bounded in-memory obstacle vectors and swept segment checks, with no allocation per obstacle beyond normal iteration.
- Regression: existing facade, arena config, tick-limit, and command rejection tests still pass.
- Manual UI checks: not applicable; no UI changed.

Not tested and why:

- Network, protocol, backend, and UI flows are not touched by this task.
- Hazard collision is not tested yet because hazard gameplay is scheduled for task 0020.

Known risks:

- Collision is cell-obstacle based for the MVP map. More advanced continuous geometry can be added later if design needs it.

Ready for verification: yes.

## Verification-Agent Output

State transition: `implementing -> verifying`.

Diff scope:

- Changed files are limited to `src/battle_core/**`, `tests/unit/core/**`, and `docs/agent-runs/**`.
- Forbidden transport, backend, Qt, frontend, and external paths are unchanged.
- Build outputs remain ignored and unstaged.

Commands/results:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- `git diff --check` - passed, with line-ending warnings only.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `C:\Program Files\Git\bin\bash.exe -lc 'python3() { "/c/Users/user/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe" "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'` - passed.

Verification decision: approve.

## Required Review-Agent Outputs

### Architecture-Agent

Decision: approve.

Findings:

- [info] Movement, collision, base state, and coordinate transforms stay in `battle_core` and do not introduce forbidden dependencies.
- [info] Clients still submit intentions only; there is no API for client-owned position, HP, score, team mutation, objective state, or match result.
- [info] Snapshot additions are pure game data and remain suitable for backend fanout.

Required re-checks:

- None beyond Verification-Agent checks already run.

### Performance-Agent

Decision: approve.

Findings:

- [info] The tick movement path iterates bounded vectors of pending commands, players, obstacles, and base zones with no per-obstacle dynamic allocation.
- [info] Swept obstacle checks prevent high-speed obstacle tunneling while keeping the MVP hot path simple and deterministic.
- [info] The current O(players * obstacles) shape is acceptable for the 21x13 MVP arena; larger maps can add spatial indexing later if load tests require it.

Required re-checks:

- None beyond Verification-Agent checks already run.

## Verification-Agent After Reviews

State transition: `verifying -> reviewing -> commit-ready`.

Review status:

- Architecture-Agent: approve.
- Performance-Agent: approve.

No Fix-Agent was needed.

## Commit-Agent Output

State transition: `commit-ready -> committed`.

Commit policy:

- One scoped task commit.
- No forbidden files.
- No generated dependency directories or build outputs.
- Secret scan passed before commit.

Commit:

- `core: add player movement collision`
- Final amended commit hash is recorded in the Coordinator closeout response.

## Coordinator Closeout

Final status: committed.

Known risks:

- Collision is cell-obstacle based for the MVP map. More advanced continuous geometry can be added later if design needs it.
- Hazard collision is intentionally deferred to task 0020.

Follow-up:

- Start task `0019-core-objective-run-rules.md` in a new task branch after this branch is merged/reviewed as appropriate.
