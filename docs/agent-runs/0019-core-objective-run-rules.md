# Task 0019: Core Objective Run Rules

State: `implementing`

Branch: `agent/0019-core-objective-run-rules`

## Coordinator Preflight

Task packet: `docs/agent-tasks/0019-core-objective-run-rules.md`.

Sequential order:

- Previous task `0018-core-player-movement-and-collision` is terminal and committed on its task branch.
- Current task branch was created from that sequential base.

Allowed files:

- `src/battle_core/**`
- `tests/unit/core/**`
- `docs/game/**`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `docs/agent-runs/`

Forbidden files:

- `src/battle_transport_*/*`
- `src/battle_qt_client/**`
- `frontend/**`
- `deploy/**`
- `external/**`

Required gates and reviews:

- Gate A: architecture boundaries.
- Gate C: deterministic gameplay tests.
- Gate G: tick-loop state changes.
- Gate K: sequential pipeline.
- Gate L: playable scenario integrity.
- Architecture-Agent: required.
- Security-Agent: required for command validation.
- Performance-Agent: required because tick-loop state changes are touched.
- Verification-Agent: required.

Implementation-Agent prompt:

```text
Implement task 0019 only. Add Objective Run state and lifecycle in battle_core: at spawn, dropped, carried, respawning, captured; interact pickup validation; carrier slowdown; server-side drop hook with pickup lock; capture at own base only; score tracking; configured win condition; objective events; deterministic tests for Scenario A path, pickup lock, enemy-base failure, carrier slowdown, respawn, and authority validation. Do not edit forbidden paths and do not commit.
```

## Implementation-Agent Output

State transition: `ready -> implementing`.

Assumptions:

- The drop hook is a core/server-side facade for later combat, defeat, and hazard systems; clients still cannot command objective drop directly.
- Objective timings are represented in deterministic core ticks for this task.
- Hazard-triggered drops are deferred to task 0020.

Changed files:

- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/README.md`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `docs/agent-runs/0019-core-objective-run-rules.md`

Implemented:

- Added objective config, objective snapshot state, team scores, and objective event types.
- Added `interact` pickup validation using objective state, pickup lock, and pickup radius.
- Added carrier slowdown during movement while carrying the objective.
- Added server-side `dropObjective()` with pickup lock and dropped event emission on the next tick.
- Added capture validation against the carrier's own base only.
- Added score increments, respawn delay, objective respawn, and score-limit match finish.
- Updated battle_core README with the Objective Run boundary.

Checks run during implementation:

- `cmake --build build --parallel` - passed after implementation fixes.
- `ctest --test-dir build --output-on-failure` - initially failed due a test setup that expected capture before the slowed carrier reached base; passed after fixing the helper.
- Pre-review validation fixes for finished-match drops and zero-tick respawn delay were added; `cmake --build build --parallel` and `ctest --test-dir build --output-on-failure` passed afterward.

Test Impact Matrix:

Changed behavior:

- `BattleEngine` now supports the Objective Run lifecycle, including pickup, carried, dropped, respawning, captured, score, and win condition.
- `interact` is now implemented as a server-validated objective pickup intention when objective mode is enabled.

Tests added/updated:

- Happy path: deterministic Objective Run capture increments score, emits pickup/capture/score/match-finished events, and ends at score limit.
- Corner cases: objective respawns after configured capture delay when score limit is not reached.
- Invalid input / hostile input: immediate re-pickup during pickup lock is rejected; capture at enemy base fails.
- Authority / ownership: clients still cannot set objective state, score, match result, or position directly; unsupported authority commands remain rejected.
- Resource bounds / performance: objective state updates are constant-time except existing bounded player/base lookups.
- Regression: existing movement, collision, arena config, tick-limit, and command rejection tests still pass.
- Manual UI checks: not applicable; no UI changed.

Not tested and why:

- Hazard, combat, defeat, and hit-triggered drops are not tested because task 0020 owns those systems.
- Network/protocol/backend authority checks are not tested because this task is core-only.

Known risks:

- Objective events are still core-local; later protocol/backend tasks must expose only server-owned snapshots/events.

Ready for verification: yes.

## Verification-Agent Output

State transition: `implementing -> verifying`.

Diff scope:

- Changed files are limited to `src/battle_core/**`, `tests/unit/core/**`, and `docs/agent-runs/**`.
- Forbidden transport, Qt, frontend, deploy, and external paths are unchanged.
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

- [info] Objective state, scores, and events remain in `battle_core` and do not add transport, UI, Telegram, deploy, or external dependencies.
- [info] `interact` is still an intention; core validates pickup state, distance, and lock before mutating objective state.
- [info] Capture uses server-owned team/base state and does not allow client-owned score, match result, or objective state.

Required re-checks:

- None beyond Verification-Agent checks already run.

### Security-Agent

Decision: approve.

Findings:

- [info] Command validation rejects unknown players, unsupported attack authority, invalid pickup state, pickup lock, enemy-base capture, and finished-match drops.
- [info] Objective config values are validated for bounds, positive pickup radius, valid carrier speed multiplier, and non-zero score limit.
- [info] No network, parser, logging, secrets, or raw payload surface changed in this core-only task.

Required re-checks:

- None beyond Verification-Agent checks already run.

### Performance-Agent

Decision: approve.

Findings:

- [info] Objective updates are constant-time per tick except existing bounded player/base/vector scans.
- [info] Carrier slowdown reuses the existing movement path and does not add unbounded queues or allocations in the tick loop.
- [info] Event vectors remain per-tick outputs and are not retained unboundedly.

Required re-checks:

- None beyond Verification-Agent checks already run.

## Verification-Agent After Reviews

State transition: `verifying -> reviewing -> commit-ready`.

Review status:

- Architecture-Agent: approve.
- Security-Agent: approve.
- Performance-Agent: approve.

No Fix-Agent was needed after required reviews.

## Commit-Agent Output

State transition: `commit-ready -> committed`.

Commit policy:

- One scoped task commit.
- No forbidden files.
- No generated dependency directories or build outputs.
- Secret scan passed before commit.

Commit:

- `core: add objective run rules`
- Final amended commit hash is recorded in the Coordinator closeout response.

## Coordinator Closeout

Final status: committed.

Known risks:

- Objective events are still core-local; later protocol/backend tasks must expose only server-owned snapshots/events.
- Hazard, combat, defeat, and hit-triggered drops are intentionally deferred to task 0020.

Follow-up:

- Start task `0020-core-combat-dash-and-hazards.md` in a new task branch after this branch is merged/reviewed as appropriate.
