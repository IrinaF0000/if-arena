# Run 0076: Corrective crow patrol radius

## Status

State: commit-ready
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0076-corrective-crow-patrol-radius.md`
Implementation commit: pending

## Scope

- Core crow patrol semantics, scenario config parsing, deterministic unit coverage, task/run documentation.
- No workflow, deploy, transport envelope, client authority, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- `range` remains the authoritative hazard effect distance exposed to clients.
- `patrolRadius` is gameplay configuration for server-side crow movement and should not be shown in normal arena overlays.

## Test Impact Matrix

Changed behavior:
- Crow patrol movement now uses `patrolRadius` instead of hazard effect `range`.
- Scenario config parses optional `patrolRadius` and the default scenario sets it for `center_crow`.
- Invalid crow patrol radii shorter than one map cell are rejected.

Tests added/updated:
- Happy path: core test proves a crow with `range: 0.9` and `patrolRadius: 1.5` changes patrol cells.
- Corner cases: core test verifies the crow remains within patrol radius while snapshot effect `range`/`radius` stay unchanged.
- Invalid input / hostile input: backend scenario parser test rejects `patrolRadius: 0.9`; core constructor and arena validation reject impossible crow patrols.
- Authority / ownership: no client authority or transport envelope behavior changed; clients still receive effect metadata only.
- Resource bounds / performance: patrol selection still scans the same fixed six candidate offsets per crow.
- Regression: core/backend tests and scenario validators passed.
- Manual UI checks: final corrective visual acceptance still needs real desktop/browser screenshots.

Not tested and why:
- Manual visible crow movement is deferred to the final corrective UI acceptance pass; this task adds deterministic engine/parser coverage.

## Checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed with optional Vulkan header warning only.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure -R "battle_core|battle_backend"` - passed, 2/2 tests.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/ci/validate_no_hardcoded_scenarios.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed.

## Reviews

- Verification-Agent: approved; checks passed and diff stayed inside task scope.
- Architecture-Agent: approved; `battle_core` remains dependency-clean, `patrolRadius` is value-type deterministic config, and clients/protocol remain effect-range based.
- Security-Agent: approved; scenario parser treats `patrolRadius` as untrusted input and rejects invalid finite/range values before gameplay use.

## Risks

- Final visual confirmation that crows move in the actual desktop/browser arena remains part of later corrective acceptance.
