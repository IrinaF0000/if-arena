# Task 0020: Core Combat Dash And Hazards

State: `implementing`

Branch: `agent/0020-core-combat-dash-and-hazards`

## Coordinator Preflight

Task packet: `docs/agent-tasks/0020-core-combat-dash-and-hazards.md`.

Sequential order:

- Tasks `0016` through `0019` are merged into `master` in order with post-merge checks.
- Current task branch was created from updated `master`.

Allowed files:

- `src/battle_core/**`
- `tests/unit/core/**`
- `config/scenarios/**`
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
- Gate G: tick-loop combat/hazard path.
- Gate K: sequential pipeline.
- Gate L: playable scenario integrity.
- Architecture-Agent: required.
- Security-Agent: required.
- Performance-Agent: required.
- Verification-Agent: required.

Implementation-Agent prompt:

```text
Implement task 0020 only. Add MVP attack, cooldown, dash, and deterministic mine/tower hazards to battle_core. Clients may send only attack/dash intentions with directions; core owns hit detection, cooldowns, damage, dash distance, collision-safe dash movement, hazard timing, and objective drop on carrier hit. Add focused deterministic tests for cooldowns, invalid attacks, dash bounds, hazard hits, and drop-on-hit. Do not edit forbidden paths and do not commit.
```

## Implementation-Agent Output

State transition: `ready -> implementing`.

Assumptions:

- Attack and dash reuse `PlayerCommand` direction as an intention; no client-provided hit, damage, cooldown, or dash distance is accepted.
- Mines and towers share a small deterministic hazard DTO for this MVP; richer hazard behavior can be split later if needed.
- Hazard configuration already exists in the public scenario JSON, so no JSON parser or config file rewrite is needed in this task.

Changed files:

- `src/battle_core/BattleEngine.hpp`
- `src/battle_core/BattleEngine.cpp`
- `src/battle_core/README.md`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `docs/agent-runs/0020-core-combat-dash-and-hazards.md`

Implemented:

- Added combat config for attack damage/range/cooldown and dash distance/cooldown.
- Added attack and dash command constructors and validation.
- Added player attack/dash cooldown snapshot fields.
- Added server-owned attack hit detection, damage application, miss events, and cooldown enforcement.
- Added collision-safe dash movement with arena bounds and obstacle checks.
- Added mine/tower hazard config/snapshots with deterministic telegraph and hit events.
- Added objective drop when the carrier is hit by combat or hazards.
- Documented the battle_core combat/dash/hazard authority boundary.

Checks run during implementation:

- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- After review hardening fixes, `cmake --build build --parallel` and `ctest --test-dir build --output-on-failure` passed again.

Test Impact Matrix:

Changed behavior:

- `BattleEngine` now supports attack, dash, combat cooldowns, damage, MVP mine/tower hazards, and objective drop on carrier hit.
- Snapshots expose attack/dash cooldowns and hazard state.

Tests added/updated:

- Happy path: valid attack hits an opponent and applies server-owned damage.
- Corner cases: dash clamps to arena bounds while setting server-owned cooldown.
- Invalid input / hostile input: zero and oversized attack aims are rejected; duplicate same-tick attack is rejected by cooldown policy.
- Authority / ownership: clients still cannot decide hit, damage, cooldown, dash distance, objective drop, hazard hit results, or defeated-player actions.
- Resource bounds / performance: combat and hazard paths use bounded vectors and deterministic scans; unbounded dash distances and excessive hazard counts are rejected.
- Regression: existing movement, objective, arena, and tick-limit tests still pass.
- Manual UI checks: not applicable; no UI changed.

Not tested and why:

- Protocol/backend/client representation of combat events is deferred to later protocol and integration tasks.
- Drone hazards remain out of MVP scope; only mine and tower behavior is modeled.

Known risks:

- Hazard target selection is first matching living player in config/vector order. This is deterministic and adequate for MVP, but balancing may require richer targeting later.

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

Initial verification decision: request-changes.

Findings:

- [medium] Defeated players could still have commands accepted at submit-time, even though tick processing ignored them.
- [medium] Hostile config could set an unbounded dash distance, causing too many collision-sweep steps.

## Required Review-Agent Outputs

### Architecture-Agent

Decision: approve after Fix-Agent hardening.

Findings:

- [info] Combat, dash, hazard state, cooldowns, damage, and objective drop remain in `battle_core` with no transport/UI/backend dependencies.
- [info] Commands remain intentions only; hit, damage, cooldown, dash distance, and objective drop are resolved by core-owned config/state.
- [info] Snapshot/event additions are pure game data and keep backend/client authority boundaries intact.

Required re-checks:

- Re-run C++ and repo checks after hardening fixes.

### Security-Agent

Decision: approve after Fix-Agent hardening.

Findings:

- [medium] Defeated players must be rejected at command submission, not only ignored during tick processing.
- [medium] Dash distance and hazard counts should be bounded because match config is untrusted input.
- [info] Attack aim rejects zero/oversized directions; duplicate same-tick attacks are rejected.

Required re-checks:

- Re-run C++ tests, `git diff --check`, secret scan, and harness validation.

### Performance-Agent

Decision: approve after Fix-Agent hardening.

Findings:

- [medium] Dash sweep step count must be bounded by validated config.
- [info] Combat and hazard scans are O(players + hazards) over bounded vectors and are acceptable for the MVP arena.
- [info] The tick path does not introduce persistent unbounded queues or per-frame heap ownership tricks.

Required re-checks:

- Re-run C++ tests after bounding dash distance and hazard count.

## Fix-Agent Output

State transition: `reviewing -> fixing`.

Fixed findings:

- `submit()` now rejects commands from defeated players.
- Combat config rejects dash distances larger than the arena extent.
- Match config rejects hazard counts larger than the arena cell count.
- Added tests for defeated-player command rejection and unbounded dash config rejection.

Changed files:

- `src/battle_core/BattleEngine.cpp`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `docs/agent-runs/0020-core-combat-dash-and-hazards.md`

Remaining risks:

- Hazard target selection remains first matching living player in deterministic vector order.

Ready for re-verification: yes.

## Verification-Agent After Fixes

State transition: `fixing -> re-verifying`.

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

State transition: `re-verifying -> commit-ready`.

## Commit-Agent Output

State transition: `commit-ready -> committed`.

Commit policy:

- One scoped task commit.
- No forbidden files.
- No generated dependency directories or build outputs.
- Secret scan passed before commit.

Commit:

- `core: add combat dash hazards`
- Final amended commit hash is recorded in the Coordinator closeout response.

## Coordinator Closeout

Final status: committed.

Known risks:

- Hazard target selection is first matching living player in config/vector order. This is deterministic and adequate for MVP, but balancing may require richer targeting later.
- Protocol/backend/client representation of combat and hazard events is deferred to later tasks.

Follow-up:

- Merge task `0020` into `master` after post-commit sanity checks, then start task `0021-playable-gameplay-protocol-messages.md` on a new branch.
