# Task 0037: GitHub CI and public release baseline

## Goal

Prepare the repository for stable public GitHub development without pushing, tagging, deploying, or weakening CI/security checks.

## Scope

Allowed files:

- `.github/workflows/pr-ci.yml`
- `.github/workflows/main-ci.yml`
- `README.md`
- `.gitignore`
- `LICENSE`
- `docs/ci/CI_CD_GUARDRAILS.md`
- `docs/operations/RELEASE_NOTES_v0.2.1-playable-alpha.md`
- `docs/agent-runs/0037-github-ci-and-public-release-baseline.md`
- `docs/agent-tasks/0037-github-ci-and-public-release-baseline.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `src/**`
- `frontend/**`
- `deploy/**`
- production secrets
- release tags
- pushing to GitHub
- task 0038 or later

## Required behavior

- PR CI remains validation-only and secret-free.
- Main CI remains separate from PR CI.
- Architecture-boundary validator is explicit in PR and main CI.
- README has a public CI badge and current local demo/test instructions.
- LICENSE and `.gitignore` are verified; only update them if a gap is found.
- Release notes exist for the current public baseline and record the recommended tag without creating it.

## Required quality gates

- Gate A. Every implementation task
- Gate H. Documentation-only changes
- Gate I. CI/CD and workflow changes
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cd frontend/telegram_mini_app && npm.cmd run typecheck`
- `cd frontend/telegram_mini_app && npm.cmd run lint`
- `cd frontend/telegram_mini_app && npm.cmd run build`
- `git diff --check`

## Required reviews

- Coordinator: yes
- Security-Agent: yes
- Verification-Agent: yes
- CI/CD-Agent or Coordinator workflow review: yes
- Architecture-Agent: yes for boundary validator CI wiring

## Acceptance criteria

- CI workflows contain explicit architecture-boundary validation.
- Workflow permissions remain least-privilege.
- PR workflow does not deploy and does not use production secrets.
- Main workflow does not deploy automatically.
- README is suitable for a public visitor.
- Release notes list final checks, known limitations, and a recommended tag.
- No tag or push is performed.

## Rollback note

Revert this task commit to restore previous workflow files, README/release notes, and task docs. Branch protection check names are unchanged.
