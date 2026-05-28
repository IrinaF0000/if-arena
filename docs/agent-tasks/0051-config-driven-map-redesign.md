# Task 0051: Config-driven map redesign

## Goal

Redesign the playable Objective Run arena only through scenario config and lock map fairness with config-driven validation tests.

## Scope

Allowed files:

- `config/scenarios/arena_small_objective_run.json`
- `scripts/ci/validate_scenario_map_fairness.py`
- `tests/ci/validate_scenario_map_fairness_tests.py`
- `docs/agent-runs/0051-config-driven-map-redesign.md`
- `docs/agent-tasks/0051-config-driven-map-redesign.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay rules
- protocol schemas
- Qt client source
- Telegram Mini App source
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Keep the arena at the 21x13 MVP size.
- Preserve 180-degree rotational symmetry.
- Keep the objective central and the whole arena visible.
- Move layout interest into scenario config; do not duplicate the new map in C++ or test code.
- Avoid a single central obstacle/hazard line.
- Keep at least two viable routes from each spawn to the objective and from objective to each base.
- Avoid required one-cell choke points.
- Place hazards around route choices, not all on the objective line.

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
- `python tests/ci/validate_scenario_map_fairness_tests.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, for public scenario config and validation script behavior
- Performance-Agent: no
- Frontend-Agent: no
- Qt-Agent: no
- CI/CD-Agent: yes, for new `scripts/ci/**` validator
- Verification-Agent: yes

## Acceptance criteria

- The default playable map is more interesting while still symmetric and readable.
- Pathability and no-required-choke validation loads map roles from scenario config.
- Symmetry validation covers obstacles, hazards, bases, spawns, and the central objective.
- Existing paired desktop/mobile gameplay scenario tests still pass.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to return to the previous playable map layout and remove the fairness validator.
