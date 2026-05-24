# Task 0038: Add browser E2E Playwright harness

## State

ready for commit

## Coordinator Preflight

- Branch: `agent/0038-browser-e2e-playwright-harness`.
- Baseline: `master` after local merge of task 0037.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0038-browser-e2e-playwright-harness.md`.
- Allowed implementation scope confirmed: frontend Playwright config/tests/package files, frontend README, task packet, run note, task README.
- Forbidden scope confirmed: no source/server/protocol behavior changes, workflows, deploy files, release tags, push, or task 0039 work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0038. Add Playwright E2E setup for the Telegram Mini App, add `test:e2e` scripts, create a two-client WebSocket browser scenario that catches invalid envelopes, idle timeouts, passive snapshot failures, and console/page errors. Document Windows commands. Do not change server gameplay/protocol behavior or start task 0039.

## Implementation-Agent

- Added Playwright under `frontend/telegram_mini_app` with `test:e2e` and `test:e2e:headed` scripts.
- Added `playwright.config.ts` that starts Vite on `127.0.0.1:5174`, does not reuse stale local servers, retains traces on failure, and targets Chromium.
- Added `tests/e2e/two-client-websocket.spec.ts`.
- The E2E starts `build/battle_server_app.exe` with a temporary WebSocket-only local config on the client default port `8081`, opens two browser clients, creates and joins a match, moves one client while the other is passive, and verifies both receive snapshots beyond idle timeout.
- The test captures page console/page errors and game WebSocket frames, checks for `invalid_envelope` and `idle_timeout`, and keeps Vite HMR WebSocket frames out of gameplay assertions.
- Documented Windows commands and browser install command in the Mini App README.
- `package-lock.json` was generated during local dependency installation but left out of the commit because the current repository secret scanner flags npm integrity strings as high-entropy literals. The repository did not previously track this frontend lockfile, and changing `scripts/ci/**` is outside task 0038 scope.

## Test Impact Matrix

Changed behavior:

- New local browser E2E coverage for the Telegram Mini App WebSocket playable path.
- New npm scripts for headed/headless Playwright runs.

Tests added/updated:

- Happy path: two real browser clients authenticate, create/join a match, and receive live snapshots.
- Corner cases: passive client stays connected and receives snapshots beyond the configured idle timeout.
- Invalid input / hostile input: asserts no server `invalid_envelope` or `idle_timeout` error frames during the browser flow.
- Authority / ownership: browser clients send only protocol intentions to the authoritative server; no gameplay/server behavior changed.
- Resource bounds / performance: server config keeps message, pending queue, command rate, and match/client limits explicit.
- Regression: covers the WebSocket envelope and passive-timeout regressions fixed before 0038.
- Manual UI checks: not required; this task adds automated browser coverage and keeps traces on E2E failure.

Not tested and why:

- `VITE_WS_URL` override behavior is not changed or asserted because `src/**` is forbidden in this task packet. The E2E uses the current public local fallback `ws://127.0.0.1:8081/ws`.
- GitHub-hosted CI was not run because no push was performed.

Ready for verification: yes.

## Verification-Agent

- `npm.cmd run test:e2e` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed.
- `node tests\frontend\telegram_protocol_validation.mjs` - passed.
- `node tests\frontend\telegram_websocket_client_behavior.mjs` - passed.
- `node tests\frontend\telegram_arena_canvas_assets.mjs` - passed.
- `python tests\frontend\websocket_local_smoke.py` - passed.
- `cmake --build build --parallel` - passed.
- `python scripts\ci\validate_architecture_boundaries.py` - passed.
- `python scripts\ci\scan_secrets.py` - passed after omitting generated `package-lock.json`.
- `python scripts\agent\validate_agent_harness.py` - passed.
- `python scripts\agent\validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed.
- `scripts/ci/validate_structure.sh` through Git Bash shim - passed.

## Review-Agent

- Security Review-Agent: required and completed. No secrets committed; no auth/session or server protocol behavior changed. E2E treats network input through the existing strict client/server protocol path.
- Architecture Review-Agent: required and completed. The task stays inside frontend test/config/docs/package scope and does not add transport, UI, or Telegram code to `battle_core`.
- Verification Review-Agent: required and completed. Required checks passed locally; generated Playwright traces and reports were not staged.

## Fix-Agent

- Fixed E2E stability after initial failure by replacing a raw TCP readiness probe with a valid WebSocket upgrade probe.
- Fixed local determinism by disabling Playwright web server reuse.
- Aligned the temporary C++ WebSocket server with the current client default port `8081` because source changes to runtime URL configuration are forbidden in this task.

## Commit-Agent

- Commit created with message `test: add browser e2e websocket harness`.
- Final commit hash is recorded in the coordinator response because amending this file changes the hash.
- Scope checked before commit: only task 0038 files were staged; root roadmap file remained unstaged.

## Coordinator Closeout

- Task 0038 is complete on branch `agent/0038-browser-e2e-playwright-harness`.
- Final commit hash is recorded in the coordinator response.
- Ready for local merge to `master`.
- No push or release tag was performed.
