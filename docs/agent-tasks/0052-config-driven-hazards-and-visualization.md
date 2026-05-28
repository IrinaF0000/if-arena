# Task 0052: Config-driven hazards and visualization

## Goal

Expose hazard behavior and presentation metadata from scenario config through snapshots, then render hazard range, icon identity, cooldown state, and concise legend details in both clients.

## Scope

Allowed files:

- `config/scenarios/arena_small_objective_run.json`
- `config/test_scenarios/*.json`
- `assets/svg/*.svg`
- `src/battle_core/**`
- `src/battle_backend/**`
- `src/battle_qt_client/**`
- `frontend/telegram_mini_app/src/**`
- `docs/architecture/PROTOCOL.md`
- `tests/unit/core/**`
- `tests/unit/backend/**`
- `docs/agent-runs/0052-config-driven-hazards-and-visualization.md`
- `docs/agent-tasks/0052-config-driven-hazards-and-visualization.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Scenario hazards must define id, kind, position, radius, range, effect type, cooldown duration, trigger condition, and icon id.
- Core rules must consume config-provided radius/range/effect/cooldown/trigger metadata.
- Snapshot serialization must expose hazard metadata and dynamic state.
- Qt and Telegram Mini App clients must validate/render hazard range circles and icon-driven markers without client-side gameplay authority.
- Add project-owned SVG hazard icons in a consistent simple style.
- Add or update tests for hazard metadata parsing, snapshot serialization, dynamic cooldown state, and invalid hazard metadata.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_scenario_map_fairness.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, for scenario metadata parsing and frontend message validation
- Performance-Agent: yes, for per-snapshot metadata/render cost
- Frontend-Agent: yes
- Qt-Agent: yes
- CI/CD-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Hazard metadata is authored in scenario config and visible in snapshots.
- Both clients draw server-provided range metadata and distinguish icon ids.
- Hazard legend text is derived from snapshot metadata.
- Existing paired desktop/mobile scenario tests still pass.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to return hazards to kind-only snapshot rendering.
