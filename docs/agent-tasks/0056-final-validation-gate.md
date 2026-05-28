# Task 0056: Final validation gate

## Goal

Run the post-0046 final validation gate on `agent/0047-playable-stabilization` before any push, tag, or merge decision.

## Scope

Allowed files:

- `README.md`
- `docs/agent-runs/0056-final-validation-gate.md`
- `docs/agent-tasks/0056-final-validation-gate.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- source code
- scenario configs
- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Run the final backend/core, frontend, smoke, gameplay, architecture, security, and agent validators from the post-0046 plan.
- Use clean `build` and `build-werror` directories.
- Record exact pass/fail results and any approved skipped checks.
- Fix narrow documentation inaccuracies discovered during validation.
- Do not change implementation code while running this gate.

## Required quality gates

- Gate H. Documentation-only changes
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-werror -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
- `cmake --build build-werror --parallel`
- `bash scripts/ci/validate_structure.sh`
- `python scripts/ci/scan_secrets.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python tests/ci/validate_architecture_boundaries_tests.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `npm install`
- `npm run typecheck`
- `npm run lint`
- `npm run build`
- `python tests/integration/server/tcp_vertical_slice_smoke.py`
- `python tests/integration/server/tcp_live_tick_smoke.py`
- `python tests/frontend/websocket_local_smoke.py`
- desktop and mobile config-driven gameplay scenario suites
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- All required final gate checks pass or blockers are explicitly recorded.
- No generated artifacts are staged.
- No push, tag, or merge is performed.

## Rollback note

Revert the validation run-note/task-packet commit if the recorded gate summary must be replaced.
