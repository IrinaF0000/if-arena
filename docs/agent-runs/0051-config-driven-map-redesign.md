# Run 0051: Config-driven map redesign

## Status

State: committed
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0051-config-driven-map-redesign.md`

## Scope

- Redesign `config/scenarios/arena_small_objective_run.json` only through scenario data.
- Add config-driven pathability and symmetry validation for playable scenario maps.
- Do not modify source gameplay rules, client code, workflows, deploy files, or protected CI docs.

## Progress

- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Test Impact Matrix

Changed behavior:
- The default playable Objective Run map now uses staggered symmetric cover and off-center side hazards while keeping the central objective lane readable.
- A new config-driven fairness validator checks playable scenario map pathability, route comparability, no required one-cell choke points, and rotational symmetry.

Tests added/updated:
- Happy path: `python scripts/ci/validate_scenario_map_fairness.py` validates the repo scenario; paired desktop/mobile full-capture scenario tests pass on the redesigned map.
- Corner cases: `python tests/ci/validate_scenario_map_fairness_tests.py` covers central objective/self-mirroring hazard behavior through the valid scenario path and route chokepoint detection.
- Invalid input / hostile input: self-tests reject asymmetric obstacles and unmatched hazards.
- Authority / ownership: None expected; no client authority behavior changed.
- Resource bounds / performance: validator uses bounded grid BFS over 21x13 scenario maps; no runtime server resource behavior changed.
- Regression: `ctest --test-dir build --output-on-failure`, no-hardcoded-scenarios validator, gameplay pair validator, and both TCP/WebSocket scenario tests passed.
- Manual UI checks: None expected; config/test-only task.

Not tested and why:
- Manual visual UI: not run because this task changes server-authored scenario config and validation only; existing desktop/mobile gameplay scenario tests cover the playable route through real transports.

## Checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/mobile/objective_run_full_capture_mobile.py` - passed.
- `python scripts/ci/validate_no_hardcoded_scenarios.py` - passed.
- `python scripts/ci/validate_scenario_map_fairness.py` - passed.
- `python tests/ci/validate_scenario_map_fairness_tests.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed with CRLF normalization warning for `docs/agent-tasks/README.md`.

## Reviews

- Architecture-Agent: approved; changes are limited to scenario JSON, Python validation, and task/run docs, with no subsystem boundary movement.
- Security-Agent: approved; public config remains secret-free, and validation reads local repo scenario files only.
- Performance-Agent: not required.
- Frontend-Agent: not required.
- Qt-Agent: not required.
- CI/CD-Agent: approved; new validator is advisory repo automation under `scripts/ci/**` with focused self-tests and no workflow edits.
- Verification-Agent: approved; required checks passed and Test Impact Matrix is complete.

## Risks

- The map remains authored in config only; future UI visual polish may still be needed for hazard readability.
- No merge to `master` or `main`, push, or tag was performed.
