# 0013: Maintain CI/CD guardrails

## Phase

Phase 0: Repository bootstrap / recurring maintenance

## Goal

Maintain safe PR and main CI workflows without weakening repository protections.

## Background

The project uses Codex worktrees and agent branches. CI/CD files are security-sensitive and must not be changed as incidental feature work.

## Allowed files

- `.github/workflows/**`
- `.github/CODEOWNERS.example`
- `scripts/ci/**`
- `docs/ci/**`
- `docs/review/**`
- `docs/agent-seeds/**`

## Forbidden files

- `src/**` unless the CI task explicitly requires a tiny placeholder fix.
- `frontend/**` unless the CI task explicitly requires a package-script fix.
- `external/battle_simulation_snapshot/**`
- real secrets or credential files.

## Required quality gates

- Gate A
- Gate F if secrets/deployment/auth are affected
- Gate H if docs-only
- Gate I

## CI/CD impact

High. This task touches workflow behavior and repository safety.

## Security impact

Medium to high. Ensure PR workflows do not expose secrets and no deployment occurs from untrusted code.

## Performance/scalability impact

Low. CI should remain reasonably fast and avoid unnecessary heavy load tests on every PR.

## Architecture impact

Low unless workflow changes alter build targets or deployment strategy.

## Token budget instructions

- Read only the workflow files, this task packet, and `docs/ci/CI_CD_GUARDRAILS.md` first.
- Do not read source modules unless a CI failure points to them.
- Do not paste full workflow logs into reports.
- Summarize only failing steps and changed workflow files.

## Implementation steps

1. Read `docs/ci/CI_CD_GUARDRAILS.md`.
2. Inspect the current workflow files.
3. Make the smallest safe change.
4. Validate YAML syntax if tooling is available.
5. Update docs if behavior or required check names change.

## Required tests

- Run repository structure validation locally: `bash scripts/ci/validate_structure.sh`.
- Run CMake configure/build if C++ workflow behavior changed.
- Run frontend checks locally if frontend workflow behavior changed.

## Required docs updates

- `docs/ci/CI_CD_GUARDRAILS.md` if behavior changes.
- `docs/project/IMPLEMENTATION_PLAN.md` if phase strategy changes.

## Acceptance criteria

- PR CI remains validation-only.
- Main CI remains separate.
- Workflow permissions remain least-privilege.
- No production secrets are used in PR workflows.
- Rollback note is recorded in progress.

## Required reviews

- Coordinator: yes
- Security-Agent: yes
- Verification-Agent: yes
- CI/CD-Agent or Coordinator workflow review: yes

## Rollback note

Revert workflow file changes and restore previous required check names in branch protection settings if they were changed.
