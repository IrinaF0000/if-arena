# Task 0038: Add browser E2E Playwright harness

## Goal

Add a repeatable Browser E2E harness for the Web/Telegram local playable path so WebSocket regressions are caught without relying only on manual DevTools inspection.

## Scope

Allowed files:

- `frontend/telegram_mini_app/package.json`
- `frontend/telegram_mini_app/package-lock.json`
- `frontend/telegram_mini_app/playwright.config.ts`
- `frontend/telegram_mini_app/tests/e2e/**`
- `frontend/telegram_mini_app/README.md`
- `docs/agent-runs/0038-browser-e2e-playwright-harness.md`
- `docs/agent-tasks/0038-browser-e2e-playwright-harness.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `src/**`
- `.github/workflows/**`
- `deploy/**`
- gameplay/protocol/server behavior changes
- release tags
- pushing to GitHub
- task 0039 or later

## Required behavior

- Add Playwright setup under `frontend/telegram_mini_app`.
- Add npm scripts `test:e2e` and `test:e2e:headed`.
- Add a two-client browser scenario:
  - start the WebSocket server;
  - start Vite;
  - browser A connects and creates a match;
  - browser B connects and joins;
  - A moves;
  - B stays passive;
  - both receive snapshots;
  - no `invalid_envelope`;
  - no `idle_timeout`;
  - no unexpected console/page errors.
- Document Windows commands using `npm.cmd` and `npx.cmd`.

## Required quality gates

- Gate A. Every implementation task
- Gate E. Telegram Mini App
- Gate F. Security-sensitive changes, for browser/network regression coverage
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake --build build --parallel`
- `cd frontend/telegram_mini_app && npm.cmd run test:e2e`
- `cd frontend/telegram_mini_app && npm.cmd run typecheck`
- `cd frontend/telegram_mini_app && npm.cmd run lint`
- `cd frontend/telegram_mini_app && npm.cmd run build`
- existing frontend unit-style tests
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Frontend-Agent: yes
- Security-Agent: yes
- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- E2E test is runnable locally.
- Test captures console/page errors and WebSocket server error frames.
- Test validates passive client snapshot receipt beyond idle timeout.
- No client or test bypasses strict protocol/server validation.
- No release tag or push is performed.

## Rollback note

Revert this task commit to remove Playwright dependency, config, tests, scripts, and docs.
