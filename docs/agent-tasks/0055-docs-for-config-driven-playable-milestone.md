# Task 0055: Docs for config-driven playable milestone

## Goal

Update the focused project documentation so the post-0054 playable milestone is understandable and the config-driven scenario rules are explicit.

## Scope

Allowed files:

- `README.md`
- `docs/architecture/**`
- `docs/agent-rules/process/**`
- `docs/agent-runs/0055-docs-for-config-driven-playable-milestone.md`
- `docs/agent-tasks/0055-docs-for-config-driven-playable-milestone.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source code
- scenario configs
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Document `battle_core` isolation and config ownership without adding parallel architecture sources.
- Document that playable scenarios, maps, hazards, routes, and expected gameplay events are config-driven.
- Document protocol/backend ownership of hazard metadata and client authority limits.
- Document README run paths for backend, Qt desktop, Telegram Mini App, tests, objective flow, hazards, and scenario configs.
- Keep docs focused and link to canonical rule files instead of duplicating long rule blocks.

## Required quality gates

- Gate H. Documentation-only changes
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement

## Required checks

- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- README and focused architecture/process docs accurately describe the config-driven playable milestone.
- No source, CI/CD, deploy, generated, push, tag, or merge changes are made.
- Required docs-only checks pass or any skipped check has a documented reason.

## Rollback note

Revert this task commit to return documentation to the previous post-0054 state.
