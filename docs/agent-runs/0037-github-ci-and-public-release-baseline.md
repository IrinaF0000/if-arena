# Task 0037: GitHub CI and public release baseline

## State

committed

## Coordinator Preflight

- Branch: `agent/0037-github-ci-public-release-baseline`.
- Baseline: `master` after local merge of task 0036.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0037-github-ci-and-public-release-baseline.md`.
- Allowed implementation scope confirmed: PR/main workflow files, README, `.gitignore`/LICENSE only if gaps are found, CI guardrails, release notes, task packet, run note, and task README.
- Forbidden scope confirmed: no source/frontend/deploy changes, no production secrets, no push, no release tag, no task 0038 work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0037. Keep PR CI validation-only and main CI separate, add explicit architecture-boundary validator steps, add a public CI badge and release notes for the current baseline, verify LICENSE and `.gitignore`, and do not push, tag, deploy, or touch source/frontend/deploy files.

## Implementation-Agent

Assumptions:

- Existing PR/main workflows are already the right base; this task should tighten them rather than replace them.
- The GitHub remote is `IrinaF0000/if-arena`, so the README badge can use that public workflow URL.
- `.gitignore` already excludes build outputs, logs, archives, `node_modules`, frontend `dist`, local configs, and common secret material.
- `LICENSE` already exists and is MIT; no change is needed.

Changed files:

- `.github/workflows/main-ci.yml`
- `.github/workflows/pr-ci.yml`
- `README.md`
- `docs/agent-runs/0037-github-ci-and-public-release-baseline.md`
- `docs/agent-tasks/0037-github-ci-and-public-release-baseline.md`
- `docs/agent-tasks/README.md`
- `docs/ci/CI_CD_GUARDRAILS.md`
- `docs/operations/RELEASE_NOTES_v0.2.1-playable-alpha.md`

Implementation notes:

- Added explicit `Validate architecture boundaries` steps to PR and main docs/structure jobs.
- Kept workflow permissions at `contents: read`.
- Kept PR CI validation-only with no deploy or production secrets.
- Kept main CI separate and non-deploying.
- Added public PR CI badge to README.
- Added current release candidate notes for `v0.2.1-playable-alpha` with a recommended tag only; no tag was created.
- Updated CI guardrails to state architecture-boundary validation is required in PR and main CI.

## Test Impact Matrix

Changed behavior:

- PR and main CI now show architecture-boundary validation as an explicit workflow step.
- README now has a public CI badge and links to current release candidate notes.
- CI guardrails now include architecture-boundary validation as a required CI property.

Tests added/updated:

- Happy path: local architecture, structure, CMake/CTest, frontend typecheck/lint/build, agent harness, playable sequence, secret scan, and diff checks passed.
- Corner cases: no runtime code path changed; `.gitignore` and LICENSE were reviewed and left unchanged.
- Invalid input / hostile input: secret scan and architecture-boundary validation passed.
- Authority / ownership: no gameplay, protocol, backend, transport, or client authority behavior changed.
- Resource bounds / performance: no runtime queues, workers, timers, or buffers changed.
- Regression: existing CTest and frontend build checks passed.
- Manual UI checks: not applicable.

Not tested and why:

- GitHub-hosted workflow execution was not run from this local session because push is forbidden without explicit approval. Workflow changes were locally checked by running the same underlying commands.
- YAML schema validation with a dedicated parser was not run; workflow edits are limited to simple step additions under existing jobs.

Ready for verification: yes.

## Verification-Agent

Decision: pass.

Verified scope:

- No source, frontend, deploy, production secret, release tag, push, or task 0038 work.
- Workflow changes are limited to explicit architecture-boundary validation steps.
- README/release notes do not claim production readiness or tag creation.
- `.gitignore` and LICENSE were checked and did not require edits.

Checks run:

- `python scripts\ci\validate_architecture_boundaries.py` - pass.
- `python scripts\ci\scan_secrets.py` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- `git diff --check` - pass with Windows line-ending warnings only.
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass.
- `npm.cmd run typecheck` in `frontend/telegram_mini_app` - pass.
- `npm.cmd run lint` in `frontend/telegram_mini_app` - pass.
- `npm.cmd run build` in `frontend/telegram_mini_app` - pass with escalation for Vite/esbuild file access.
- `validate_structure.sh` through Git Bash with `python3` shim - pass.

## Review-Agent

CI/CD-Agent:

- Decision: approve.
- Findings: PR/main workflow separation is preserved; permissions remain least-privilege; no deployment or production-secret use is introduced; job names are unchanged.
- Required re-checks: structure validator and diff check.

Security-Agent:

- Decision: approve.
- Findings: secret scanning remains active; no secrets or secret-printing workflow steps were added; PR CI still does not use production secrets.
- Required re-checks: secret scan.

Architecture-Agent:

- Decision: approve.
- Findings: architecture-boundary validator is explicit in CI and remains wired into structure validation.
- Required re-checks: architecture-boundary validator.

## Fix-Agent

Not needed.

## Commit-Agent

- Commit: `ci: add public release baseline`.
- Staged scope matched the task packet.
- Untracked roadmap file was not staged.

## Coordinator Closeout

- Final status: committed.
- Commit hash: recorded by Coordinator final response after commit creation.
- No push, tag, deploy, production secret, source/frontend/deploy change, or task 0038 work was performed.
- Known risk: GitHub-hosted workflow execution still requires pushing a branch or PR, which was not done because push needs explicit approval.
