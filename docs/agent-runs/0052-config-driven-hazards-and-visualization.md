# Run 0052: Config-driven hazards and visualization

## Status

State: committed
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0052-config-driven-hazards-and-visualization.md`

## Scope

- Add hazard metadata to scenario config, core value types, backend snapshots, Qt parsing/rendering, and Telegram Mini App validation/rendering.
- Keep clients presentation-only; no client-owned gameplay rules.
- Do not edit workflows, deploy files, protected CI docs, release tags, push, or merge to `master`/`main`.

## Protocol Model

Chosen model: snapshot-only hazard metadata.

Reason: the current local playable protocol already broadcasts authoritative hazards in snapshots, and the stabilization plan prioritizes visible/readable gameplay over adding a new static metadata channel. Static fields are included alongside dynamic state in each snapshot until a later protocol split is justified.

## Progress

- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Test Impact Matrix

Changed behavior:
- Scenario hazards now carry id, radius, range, damage, effect, trigger, icon, and cooldown duration metadata.
- Core snapshots expose both static hazard metadata and dynamic cooldown/triggered state.
- Hazard trigger radius/range and drop behavior are driven by `HazardConfig` rather than only by hazard kind.
- Qt and Telegram Mini App rendering draw server-provided range circles, icon-driven hazard markers, cooldown labels, and compact hazard legends.

Tests added/updated:
- Happy path: core hazard metadata snapshot assertions, backend scenario parser metadata assertions, backend snapshot metadata serialization assertion, desktop/mobile full-capture scenarios.
- Corner cases: core test proves a `range` trigger can be used independently of hazard kind; map fairness validator still checks mirrored hazard metadata.
- Invalid input / hostile input: backend parser rejects unsupported hazard effect metadata; Qt and Telegram parsers reject missing/invalid hazard metadata fields.
- Authority / ownership: clients only render validated snapshot metadata and still send intentions only; no client-owned HP, objective, score, cooldown, or hazard state was added.
- Resource bounds / performance: metadata adds fixed per-hazard fields; Qt/Web render one bounded range circle and marker per authoritative hazard.
- Regression: default CMake/CTest, Qt MinGW build/CTest, no-hardcoded scenario validator, map fairness validator, gameplay pair validator, and desktop/mobile scenario wrappers passed.
- Manual UI checks: not run interactively; automated Qt build and frontend type/lint checks cover compile/schema paths, and live TCP/WebSocket scenario tests cover runtime protocol compatibility.

Not tested and why:
- `npm.cmd run build`: blocked by sandbox path access in esbuild (`Cannot read directory "../../../../../..": Access is denied`) and the required escalation retry was rejected by the system usage-limit review. `npm.cmd run typecheck` and `npm.cmd run lint` passed.

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
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - blocked by sandbox; escalation retry rejected by system usage-limit review.
- Qt MinGW configure with `BATTLE_BUILD_QT_CLIENT=ON` - passed.
- Qt MinGW build - passed.
- `ctest --test-dir build-qt-mingw --output-on-failure` - passed, 8/8 tests.
- `git diff --check` - passed with CRLF normalization warnings only.

## Reviews

- Architecture-Agent: approved; hazard metadata stays in pure core value types/snapshots, backend serialization remains transport-neutral, and clients remain presentation-only.
- Security-Agent: approved; scenario parser validates hazard identifiers/effect/trigger/icon values, and clients validate inbound hazard metadata before rendering.
- Performance-Agent: approved; the default scenario has five hazards, and per-snapshot/per-frame work remains bounded by authoritative hazard count.
- Frontend-Agent: approved; strict TypeScript typecheck/lint pass and rendering consumes validated snapshot metadata only.
- Qt-Agent: approved; Qt parser validates metadata and the Qt target builds/tests with MinGW.
- Verification-Agent: approved with one residual environment blocker for `npm.cmd run build`; all other required checks passed.

## Risks

- `npm.cmd run build` remains unverified in this environment due sandbox/escalation usage-limit blocker.
- No merge to `master` or `main`, push, or tag was performed.
