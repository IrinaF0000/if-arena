# Test Plan

Canonical project test plan. Agent-specific rules are in `docs/agent-rules/quality/TESTING.md`.

## MVP coverage target

No MVP feature is complete until changed behavior is covered by the Test Impact Matrix: happy path, corner cases, invalid/hostile input, authority checks, and resource bounds where relevant.

## Required suites

- Unit: deterministic scenarios for core gameplay, protocol parsing, backend ownership/queues, transport framing, Telegram auth, and coordinate transforms.
- Integration: backend/server process checks for auth/session lifecycle, create/join match, two-client command flow, snapshots/events, ownership rejection, disconnects, and TCP/WebSocket contracts.
- Smoke: short repeatable hostile-input, local playable, frontend protocol, load, and browser/WebSocket checks.
- Browser E2E: Playwright scenarios for the Telegram Mini App local playable path.
- Manual UI: Qt and Telegram manual checklists until automated UI coverage exists; include connect, join, movement, actions, disconnect/reconnect, errors, readability, and own-base-at-bottom view.
- Load/performance: repeatable normal, spam, slow-reader, malformed-input, and mixed-client scenarios; save reports under `reports/load/`.
- Coverage baseline: optional local C++ coverage through `BATTLE_ENABLE_COVERAGE`; coverage reports are informational and must not replace focused tests.

## Acceptance

All relevant tests pass, required manual checks are recorded, known gaps are documented, and no test is removed or weakened to hide a defect.
