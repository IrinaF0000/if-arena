# Task 0069: Final pre-push validation gate

## Goal

Run and record the final local validation gate for the playable pre-push milestone without changing source, CI/CD, deploy, protocol, or scenario config files.

## Scope

Allowed files:

- `docs/agent-runs/0069-final-pre-push-validation-gate.md`
- `docs/agent-tasks/0069-final-pre-push-validation-gate.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source code
- frontend source code
- protocol/schema changes
- scenario config changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Final run note records branch, HEAD, untracked/generated files, validation commands, results, and blockers.
- C++ build/test, frontend checks, protocol/frontend tests, gameplay scenario tests, CI validators, agent validators, security scan, and local smoke tests are run when available.
- Any skipped check must include a concrete reason.

## Required quality gates

- Gate A. Every implementation task
- Gate H. Documentation-only changes
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=<local Qt path>`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-werror -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
- `cmake --build build-werror --parallel`
- `python tests/integration/server/tcp_vertical_slice_smoke.py`
- `python tests/integration/server/tcp_live_tick_smoke.py`
- `python tests/frontend/websocket_local_smoke.py`
- all paired desktop/mobile gameplay scenario scripts under `tests/integration/desktop/` and `tests/integration/mobile/`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_touch_controls_behavior.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `node tests/frontend/telegram_main_layout_contract.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `bash scripts/ci/validate_structure.sh`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python tests/ci/validate_architecture_boundaries_tests.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/validate_scenario_map_fairness.py`
- `python tests/ci/validate_no_hardcoded_scenarios_tests.py`
- `python tests/ci/validate_gameplay_scenario_pairs_tests.py`
- `python tests/ci/validate_scenario_map_fairness_tests.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- Browser smoke of the built Mini App when the in-app browser is available.
- `git diff --check`
- `git status --short --branch`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Final run note shows all required checks passed or explicitly records a blocker/skipped reason.
- No source, CI/CD, deploy, generated, protocol, scenario config, push, tag, or merge changes are made.
- Only final validation task/run documentation is committed.

## Rollback note

Revert this task commit to remove the final validation record only.
