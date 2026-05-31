# Task 0079: Corrective final validation gate

## Goal

Run the final corrective validation gate and record whether the branch is ready for push/merge.

## Scope

Allowed files:

- `docs/agent-runs/0079-corrective-final-validation-gate.md`
- `docs/agent-tasks/0079-corrective-final-validation-gate.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source code changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub unless all blocking gates pass
- merging into `master` or `main` unless all blocking gates pass

## Required behavior

- Run final build, test, frontend, scenario, agent, and secret gates.
- Run clean process stop before and after local UI/e2e validation.
- Record mobile screenshot evidence path.
- Attempt desktop validation from a clean process state.
- Refuse push/merge if any blocking manual acceptance item cannot be verified.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Final corrective checklist from `if_arena_corrective_pre_push_agent_plan.md`

## Required checks

- `cmd /c scripts\run\stop_if_arena.cmd`
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=<local Qt path>`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `bash scripts/ci/validate_structure.sh`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `npm.cmd run test:e2e`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: no
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- All automated checks pass.
- Desktop manual flow is verified or explicitly blocks push/merge.
- Mobile e2e and screenshot evidence are available.
- Final status is recorded honestly.

## Rollback note

This task is validation-only; revert the run note if it needs to be superseded by a later validation run.
