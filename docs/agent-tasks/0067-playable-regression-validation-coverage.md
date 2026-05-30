# Task 0067: Playable regression validation coverage

## Goal

Add focused regression coverage for the stabilized playable Mini App layout and status contract without modifying CI/CD workflow files.

## Scope

Allowed files:

- `tests/frontend/**`
- `docs/agent-runs/0067-playable-regression-validation-coverage.md`
- `docs/agent-tasks/0067-playable-regression-validation-coverage.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay rules
- protocol/schema changes
- scenario config changes
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- A frontend regression test proves that Mini App service controls remain grouped in the match panel.
- The test proves score/objective/scenario and finished winner status text continue to come from authoritative snapshot data.
- The test proves mobile CSS retains the arena-focused shell and stable control grids.
- Existing scenario/config validators continue to pass.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- TypeScript/frontend quality rules

## Required checks

- `node tests/frontend/telegram_main_layout_contract.mjs`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/validate_scenario_map_fairness.py`
- `python tests/ci/validate_no_hardcoded_scenarios_tests.py`
- `python tests/ci/validate_gameplay_scenario_pairs_tests.py`
- `python tests/ci/validate_scenario_map_fairness_tests.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- New regression coverage fails if the match panel, compact status line, same-screen next-match action, or mobile control grids are removed.
- No workflow, deploy, gameplay rule, protocol, or scenario config files are modified.
- Required checks pass or any skipped check is explicitly justified in the run note.

## Rollback note

Revert this task commit to remove the added Mini App layout/status regression coverage.
