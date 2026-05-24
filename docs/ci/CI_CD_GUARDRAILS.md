# CI/CD Guardrails

These rules protect the repository CI/CD pipeline and apply to all humans and agents.

## Goals

- Keep every pull request reviewable and testable.
- Prevent accidental or malicious workflow changes.
- Ensure `main` is always releasable.
- Keep PR checks isolated from deployment credentials.
- Make CI failures actionable and cheap to debug.

## Protected paths

The following paths are CI/CD-sensitive:

- `.github/workflows/**`
- `.github/actions/**`
- `.github/dependabot.yml`
- `.github/CODEOWNERS`
- `scripts/ci/**`
- `deploy/**`
- `docs/ci/**`

Agents must not edit these paths unless the task packet explicitly allows CI/CD changes.

## Mandatory rule for workflow changes

Any change touching `.github/workflows/**` requires a dedicated CI/CD task packet with:

- exact workflow files allowed;
- reason for the workflow change;
- security impact;
- required secrets and permissions;
- expected PR checks;
- expected main-branch checks;
- rollback plan;
- Coordinator review;
- Security-Agent review;
- Verification-Agent review.

Implementation agents working on C++, Qt, backend, frontend, game logic, protocol, or docs must treat `.github/workflows/**` as forbidden files by default.

## PR CI policy

PR CI is for validation only.

Required properties:

- runs on `pull_request` targeting `main`;
- uses least-privilege permissions, usually `contents: read`;
- does not deploy;
- does not use production secrets;
- does not publish production artifacts;
- runs C++ configure/build/test checks when C++ or CMake files change;
- runs TypeScript typecheck/lint/build when Mini App files change;
- runs documentation/path sanity checks when docs or project structure files change;
- runs architecture-boundary validation;
- cancels older runs from the same branch to save CI minutes.

PR CI should be fast enough for frequent agent branches. Heavy load tests belong in scheduled/manual workflows or dedicated performance branches.

## Main CI policy

Main CI validates the merged state.

Required properties:

- runs on `push` to `main`;
- repeats the important PR checks on the final merged tree;
- runs architecture-boundary validation on the merged tree;
- may build release-style artifacts;
- may build Docker images;
- must not deploy automatically unless deployment is explicitly configured and protected by environment approvals;
- should upload only non-sensitive artifacts;
- should use environment-specific secrets only in jobs that require them.

Deployment from `main` should be manual or environment-protected until the project has stable operational procedures.

## Workflow security rules

- Do not use `pull_request_target` unless a dedicated security review approves it.
- Do not expose secrets to untrusted pull request code.
- Do not print secrets, tokens, Telegram bot tokens, deployment keys, or raw `initData`.
- Avoid broad permissions such as `contents: write` unless required by a dedicated release/deploy job.
- Prefer pinned action versions. For production-sensitive workflows, pin third-party actions to commit SHA.
- Keep workflow scripts small and readable. Complex logic belongs in reviewed scripts under `scripts/ci/`.
- Do not download and execute remote scripts in CI.
- Do not add deployment jobs in the same task as feature implementation.

## Branch protection recommendations

Configure these settings in GitHub UI after the repository is created:

- require pull request before merging to `main`;
- require status checks to pass before merge;
- require branches to be up to date before merge;
- require review from code owners for `.github/workflows/**` if CODEOWNERS is configured;
- restrict who can push directly to `main`;
- disallow force pushes to `main`;
- disallow deletion of `main`;
- require signed commits if desired for extra provenance.

Suggested required checks:

- `pr-cpp-build`
- `pr-telegram-mini-app`
- `pr-docs-and-structure`

Main-only release/deploy jobs should not be required for PR merge unless they are lightweight and deterministic.

## CODEOWNERS recommendation

Use `.github/CODEOWNERS` to require review for sensitive areas. Replace placeholders with real GitHub handles.

Example:

```text
.github/workflows/** @your-github-handle
.github/actions/** @your-github-handle
.github/CODEOWNERS @your-github-handle
deploy/** @your-github-handle
docs/ci/** @your-github-handle
```

Do not add invalid owners to a real repository because invalid CODEOWNERS entries may not enforce review.

## Agent workflow preservation rules

Agents must:

- read this file before editing any CI/CD-sensitive path;
- preserve workflow separation between PR validation and main validation;
- preserve least-privilege `permissions` blocks;
- preserve no-deploy-on-PR behavior;
- preserve path filters unless explicitly changing the trigger strategy;
- update this document if CI/CD behavior changes;
- add an ADR for major workflow or deployment changes.

Agents must not:

- disable CI checks to make a task pass;
- remove tests from CI without a documented replacement;
- weaken permissions to avoid a failure;
- add secrets to workflow logs;
- combine feature implementation and deployment automation in one task;
- edit workflow files as an unrelated fix.

## Rollback rule

Every CI/CD task must include a rollback note:

- previous workflow file names;
- checks expected to be restored;
- commands to validate locally;
- whether branch protection settings need to change.


## Secret scanning

- PR CI and main CI must run `python3 scripts/ci/scan_secrets.py`.
- Removing or weakening this check requires an explicit CI/CD task packet and Security Review Agent approval.
- PR CI must not depend on production secrets.
- Workflow logs must never print secret-bearing environment variables or raw auth payloads.
