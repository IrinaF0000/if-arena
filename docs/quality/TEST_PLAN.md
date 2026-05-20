# Test Plan

Canonical project test plan. Agent-specific rules are in `docs/agent-rules/quality/TESTING.md`.

## MVP coverage target

No MVP feature is complete until changed behavior is covered by the Test Impact Matrix: happy path, corner cases, invalid/hostile input, authority checks, and resource bounds where relevant.

## Required suites

- Core gameplay: deterministic scenarios for map validation, movement/collision, objective pickup/drop/capture/score, carrier slowdown, pickup lock, combat/dash/hazards, match end, and player-oriented transforms.
- Protocol: valid messages plus malformed JSON, unknown type, missing fields, wrong types, invalid enums, non-finite numbers, oversized strings/frames, partial and combined TCP frames.
- Backend integration: auth/session lifecycle, create/join match, two-client command flow, snapshots/events, ownership rejection, disconnects, slow clients, spam/rate limits, bounded queues.
- Security: invalid Telegram hash/date/user/init data, hostile payloads, client attempts to claim position, HP, cooldowns, score, team, objective state, or match result.
- UI clients: Qt and Telegram manual checklists until automated UI coverage exists; include connect, join, movement, actions, disconnect/reconnect, errors, readability, and own-base-at-bottom view.
- Load/performance: repeatable normal, spam, slow-reader, malformed-input, and mixed-client scenarios; save reports under `reports/load/`.

## Acceptance

All relevant tests pass, required manual checks are recorded, known gaps are documented, and no test is removed or weakened to hide a defect.
