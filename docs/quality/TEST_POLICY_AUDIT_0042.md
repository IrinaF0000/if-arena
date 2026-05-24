# Test Policy Audit 0042

## Test Types

- Unit tests: deterministic executable tests for one module or small module group, registered in CTest when practical.
- Integration tests: multi-module or process-level tests that exercise server/client contracts through real protocol paths.
- Smoke tests: short repeatable end-to-end checks for local playable flows, hostile input, load, or browser/WebSocket regressions.
- Browser E2E: Playwright-driven Telegram Mini App scenarios using a local Vite server and local C++ WebSocket server.
- Manual UI checks: human verification for Qt or visual polish when automated coverage is not practical yet.

## Current Suites

- C++ unit tests are registered through CTest from `CMakeLists.txt`.
- TCP integration and security smokes live under `tests/integration/server/` and `tests/security/`.
- WebSocket/browser-oriented smokes live under `tests/frontend/` and `frontend/telegram_mini_app/tests/e2e/`.
- Load and slow/spam scenarios live under `tests/load/`.
- Manual Qt checks live under `tests/manual/`.

## Run Note Audit

Older foundation run notes before the current pipeline sometimes recorded checks without the modern Test Impact Matrix headings. Current task packets and `docs/agent-rules/quality/TESTING.md` require every implementation task to include the matrix before commit.

This audit does not rewrite old historical run notes. Instead, it keeps the canonical policy forward-looking and documents the distinction between test types so future Verification-Agent and Commit-Agent passes can reject missing matrices consistently.

## Manual-Only UI Policy

Manual UI checks are acceptable only when automation is impractical for the touched behavior. The run note must explain why automation was not added, list exact manual steps, and record the result or the reason the check was not run.
