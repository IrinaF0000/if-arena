# Task 0057: Pre-push baseline inspection

## Goal

Record the baseline state before implementing `if_arena_next_pre_push_agent_plan.md`.

## Scope

Allowed files:

- `docs/agent-runs/0057-pre-push-baseline-inspection.md`
- `docs/agent-tasks/0057-pre-push-baseline-inspection.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- source code
- scenario configs
- frontend source
- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Record branch, HEAD, status, generated files, and baseline checks.
- Capture or reference current visual baseline evidence where practical.
- Do not repeat the completed post-0046 implementation work.

## Required quality gates

- Gate H. Documentation-only changes
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity

## Required checks

- `git status --short`
- `git log --oneline -10`
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `bash scripts/ci/validate_structure.sh`
- `python scripts/ci/scan_secrets.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python tests/ci/validate_architecture_boundaries_tests.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `npm install`
- `npm run typecheck`
- `npm run lint`
- `npm run build`

## Required reviews

- Verification-Agent: yes

## Acceptance criteria

- Baseline state is recorded.
- Existing checks are run and results are documented.
- No source or generated artifacts are staged.

## Rollback note

Revert this docs-only commit to remove the baseline record.
