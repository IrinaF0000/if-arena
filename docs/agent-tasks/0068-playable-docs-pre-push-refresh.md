# Task 0068: Playable docs pre-push refresh

## Goal

Refresh focused user-facing documentation for the current playable pre-push milestone after scenario consistency, rematch, object semantics, desktop layout, mobile layout, and regression coverage work.

## Scope

Allowed files:

- `README.md`
- `frontend/telegram_mini_app/README.md`
- `src/battle_qt_client/ui/README.md`
- `docs/agent-runs/0068-playable-docs-pre-push-refresh.md`
- `docs/agent-tasks/0068-playable-docs-pre-push-refresh.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source code
- protocol/schema changes
- scenario config changes
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- README describes the current playable Objective Run flow, same-screen next match, shared scenario/config source, object semantics, desktop side panel, mobile match panel, and regression checks.
- Client README files describe only presentation/client behavior and do not claim client authority over game rules.
- Documentation remains focused and does not duplicate canonical policy documents.

## Required quality gates

- Gate H. Documentation-only changes
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement

## Required checks

- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/validate_scenario_map_fairness.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Docs accurately describe current local playable desktop and Mini App flows.
- No source, CI/CD, deploy, generated, protocol, scenario config, push, tag, or merge changes are made.
- Required docs-only checks pass or any skipped check has a documented reason.

## Rollback note

Revert this task commit to return documentation to the previous playable pre-push wording.
