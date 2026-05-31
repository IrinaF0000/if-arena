# Run 0078: Corrective mobile visual e2e gate

## Status

State: commit-ready
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0078-corrective-mobile-visual-e2e-gate.md`
Implementation commit: pending

## Scope

- Mini App Playwright visual e2e assertions, screenshot artifact path, task/run documentation.
- No production source, workflow, deploy, gameplay authority, protocol/schema, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- Screenshot artifacts belong under ignored `build/agent-artifacts/` so they can be inspected locally without committing generated binary files.

## Test Impact Matrix

Changed behavior:
- Playwright e2e now runs the two-client flow in a mobile-sized browser context.
- E2E verifies the real canvas is nonblank with diverse pixels after snapshot rendering.
- E2E verifies the out-of-canvas hazard drawer updates from snapshot data.
- E2E writes screenshot evidence to `build/agent-artifacts/0078/mobile-after-snapshot.png`.

Tests added/updated:
- Happy path: real browser e2e covers connect, auth, create, join, snapshots, movement, and screenshot capture.
- Corner cases: canvas pixel check fails if the arena is blank or visually too uniform; hazard drawer check fails if metadata stays in the old canvas-only path.
- Invalid input / hostile input: e2e still asserts no `invalid_envelope` and no page errors.
- Authority / ownership: no client authority changed; browser still sends intentions only.
- Resource bounds / performance: no runtime queue/buffer behavior changed; screenshot artifact is written under ignored `build/`.
- Regression: frontend layout contract, typecheck/lint/build, e2e, CMake build, agent validators, secret scan, and diff check passed.
- Manual UI checks: screenshot file produced locally for review.

Not tested and why:
- Desktop manual screenshot flow remains in Phase 8 final validation.

## Checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed with optional Vulkan header warning only.
- `cmake --build build --parallel` - passed, no work to do.
- `node tests/frontend/telegram_main_layout_contract.mjs` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with escalation.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed before and after e2e.
- `npm.cmd run test:e2e` - passed with escalation.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warning only.

## Reviews

- Verification-Agent: approved; required checks passed and generated `test-results/` was removed after the run.
- Frontend-Agent: approved; the e2e test now verifies real mobile viewport rendering, canvas pixels, hazard drawer text, and screenshot evidence.

## Risks

- Screenshot is an ignored local artifact, not committed binary content.
