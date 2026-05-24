# Testing Rules

This is the canonical testing policy for agents. Do not duplicate it in task packets; reference it.

## Required for every non-doc task

- Add or update focused tests for each changed behavior.
- Cover the happy path and at least one relevant failure path.
- Test corner cases: boundaries, empty/missing values, duplicate/out-of-order input, timeouts, limits, invalid state transitions.
- Test invalid or hostile input for protocol, parser, config, auth, network, replay, session, and frontend message handling.
- Test authority violations: clients must not set position, HP, cooldowns, score, team, player id, objective state, or match result.
- Test resource limits when touched: frame/message size, queue bounds, rate limits, slow clients, spam clients, timers, workers, maps, buffers.
- Do not remove, weaken, or skip existing tests to make a task pass.

## Test Impact Matrix

Every implementation run note must include this matrix. `None` requires a reason.

```text
Changed behavior:
- ...

Tests added/updated:
- Happy path:
- Corner cases:
- Invalid input / hostile input:
- Authority / ownership:
- Resource bounds / performance:
- Regression:
- Manual UI checks:

Not tested and why:
- ...
```

Verification-Agent must reject the task if changed behavior has no direct test or explicit justification.
Commit-Agent must refuse implementation commits without the matrix, unless the task is docs-only or mechanical rename-only.

## Test type by area

- `battle_core`: deterministic unit tests and scenario tests.
- `battle_protocol`: valid encode/decode plus malformed, unknown, oversized, wrong-type, and invalid enum tests.
- `battle_backend`: integration tests for auth/session/match ownership, command validation, queue/rate bounds, disconnects.
- TCP/WebSocket transports: partial/combined frames, oversized payloads, malformed messages, slow readers, disconnects.
- Telegram auth/frontend: invalid hash/date/user data, no frontend secrets, inbound message validation, WebSocket state transitions.
- Qt/UI: automated tests where practical; otherwise update a manual checklist with exact steps and result.
- Load/performance: add or update repeatable scenarios when queues, workers, tick loop, broadcasts, or limits change.

## Test type definitions

- Unit tests exercise one module or tightly bounded module group with deterministic inputs.
- Integration tests exercise cross-module contracts or real local processes.
- Smoke tests are short end-to-end regressions that prove a representative local flow still works.
- Browser E2E tests drive the Telegram Mini App through Playwright and local servers.
- Manual UI checks are allowed only when automation is impractical; record exact steps, result, and why automation was not added.
- Coverage reports are informational baselines only unless a task explicitly introduces a threshold.

## Test design

Use deterministic clocks, seeds, and fake transports. Avoid flaky sleeps, real Telegram, real cloud services, production secrets, or live networks unless the task explicitly requires an integration smoke test.
