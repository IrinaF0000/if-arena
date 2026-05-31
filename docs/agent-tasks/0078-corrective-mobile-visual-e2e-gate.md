# Task 0078: Corrective mobile visual e2e gate

## Goal

Strengthen the real browser Mini App e2e test so it verifies visible mobile gameplay state and produces screenshot evidence.

## Scope

Allowed files:

- `frontend/telegram_mini_app/tests/e2e/**`
- `docs/agent-runs/0078-corrective-mobile-visual-e2e-gate.md`
- `docs/agent-tasks/0078-corrective-mobile-visual-e2e-gate.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay authority changes
- protocol/schema changes
- production source changes outside e2e support
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Playwright drives the real Mini App in a mobile-sized browser context.
- The test verifies connect/auth/create/join/snapshot from a clean local server.
- The test verifies mandatory SVG assets are served.
- The test verifies the active canvas is nonblank through browser pixel inspection.
- The test verifies hazard legend text appears outside the canvas.
- The test saves a current screenshot path for corrective acceptance evidence.
- No invalid protocol error is observed.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- TypeScript/frontend quality rules
- Testing rules

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `node tests/frontend/telegram_main_layout_contract.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `cmd /c scripts\run\stop_if_arena.cmd`
- `npm.cmd run test:e2e`
- `cmd /c scripts\run\stop_if_arena.cmd`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: no
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- The e2e test fails if the mobile canvas is blank.
- The e2e test fails if the hazard drawer is not updated from snapshot data.
- A screenshot is written under `build/agent-artifacts/0078/`.
- Required checks pass or any skipped check is explicitly justified.

## Rollback note

Revert this task commit to return the Mini App e2e test to protocol-only coverage.
