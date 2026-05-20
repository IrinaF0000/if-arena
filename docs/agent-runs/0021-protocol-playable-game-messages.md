# Task 0021: Protocol Playable Game Messages

State: `implementing`

Branch: `agent/0021-protocol-playable-game-messages`

## Coordinator Preflight

Task packet: `docs/agent-tasks/0021-protocol-playable-game-messages.md`.

Sequential order:

- Tasks `0016` through `0020` are merged into `master` in order with post-merge checks.
- Current task branch was created from updated `master`.

Allowed files:

- `src/battle_protocol/**`
- `tests/unit/protocol/**`
- `docs/architecture/PROTOCOL.md`
- `docs/project/PRODUCT_REQUIREMENTS.md`
- `docs/agent-runs/`

Forbidden files:

- `src/battle_core/**` except DTO discussion docs
- `src/battle_transport_*/*`
- `src/battle_qt_client/**`
- `frontend/**`
- `external/**`

Required gates and reviews:

- Gate A: architecture boundaries.
- Gate B: protocol schema/versioning.
- Gate C: focused protocol tests.
- Gate F: hostile network input.
- Gate K: sequential pipeline.
- Gate L: playable-game scenario integrity.
- Architecture-Agent: required.
- Security-Agent: required.
- Verification-Agent: required.

Implementation-Agent prompt:

```text
Implement task 0021 only. Extend battle_protocol with transport-independent playable message types and client payload validation for session phases. Client inputs must be intentions only: move, aim, attack, dash, interact, stop. Reject malformed, unknown, oversized, out-of-order, invalid enum, invalid direction, missing sequence, server-originated messages from clients, and client authority fields. Update protocol docs. Do not edit forbidden paths and do not commit.
```

## Implementation-Agent Output

State transition: `ready -> implementing`.

Assumptions:

- This task validates bounded envelope/payload structure; backend session identity and rate limiting are handled in later backend tasks.
- Direction components are integer unit components in `[-1, 0, 1]` for the MVP protocol boundary.
- Existing transport framing tests still cover partial/combined frame behavior; this task focuses on payload/message validation.

Changed files:

- `src/battle_protocol/Protocol.hpp`
- `src/battle_protocol/Protocol.cpp`
- `tests/unit/protocol/ProtocolEnvelopeTests.cpp`
- `docs/architecture/PROTOCOL.md`
- `docs/agent-runs/0021-protocol-playable-game-messages.md`

Implemented:

- Added playable protocol message types: `handshake`, `ping`, and `pong`.
- Added `ClientSessionPhase` and `validateClientEnvelope()`.
- Added client payload validation for auth, create match, join match, and input command messages.
- Rejected server-originated messages from clients.
- Enforced input command `sessionSeq` and session-phase order.
- Rejected client authority fields such as `hp`, `score`, `team`, `position`, cooldown/damage/objective/match-result claims.
- Validated command kind enums and direction ranges.
- Updated protocol documentation for phase validation, new message types, integer directions, and server-owned attack/dash decisions.

Checks run during implementation:

- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- After review fix for session ordering, `cmake --build build --parallel` and `ctest --test-dir build --output-on-failure` passed again.

Test Impact Matrix:

Changed behavior:

- `battle_protocol` now validates playable client message payloads and session order in a transport-independent helper.

Tests added/updated:

- Happy path: playable message types round-trip; demo auth and intent-only attack command validate.
- Corner cases: input command without `sessionSeq` is rejected.
- Invalid input / hostile input: invalid command enum, invalid direction, server-originated snapshot from client, and client authority fields are rejected.
- Authority / ownership: client payloads cannot claim HP, score, team, position, cooldown, damage, objective state, player id, or match result.
- Resource bounds / performance: parser still enforces message, payload, string, type, and request-id limits before validation.
- Regression: existing envelope parse/serialize and malformed/unknown/oversized tests still pass.
- Manual UI checks: not applicable; no UI changed.

Not tested and why:

- Backend rate limiting, auth state transitions, and command ownership by authenticated session are deferred to backend tasks.
- Frontend runtime validation is deferred to the Telegram/WebSocket slice.

Known risks:

- Payload validation is intentionally schema-focused and does not yet bind commands to a concrete backend session/player identity.

Ready for verification: yes.

## Verification-Agent Output

State transition: `implementing -> verifying`.

Diff scope:

- Changed files are limited to `src/battle_protocol/**`, `tests/unit/protocol/**`, `docs/architecture/PROTOCOL.md`, and `docs/agent-runs/**`.
- Forbidden core, transport, Qt, frontend, and external paths are unchanged.
- Build outputs remain ignored and unstaged.

Commands/results:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- `git diff --check` - passed, with line-ending warnings only.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `C:\Program Files\Git\bin\bash.exe -lc 'python3() { "/c/Users/user/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe" "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'` - passed.

Initial verification decision: request-changes.

Finding:

- [medium] `create_match`/`join_match` were allowed after the client was already in-match, and auth could be repeated after authentication.

## Required Review-Agent Outputs

### Architecture-Agent

Decision: approve after Fix-Agent order validation.

Findings:

- [info] Protocol additions stay in `battle_protocol` and remain transport-independent.
- [info] Client inputs are represented as intentions only; authority fields are rejected before backend handling.
- [medium] Session-phase message order must reject repeated auth and create/join after a match is already joined.

Required re-checks:

- Re-run protocol tests and baseline checks after order validation fix.

### Security-Agent

Decision: approve after Fix-Agent order validation.

Findings:

- [info] Unknown types, malformed JSON, missing fields, invalid field types, oversized messages/strings, invalid enum values, invalid directions, missing input sequence numbers, server messages from clients, and client authority claims are rejected.
- [medium] Message order validation must reject repeated auth and create/join after joining a match to avoid confusing session state.
- [info] Errors are structured `ProtocolError` values and do not expose parser internals.

Required re-checks:

- Re-run protocol tests, `git diff --check`, secret scan, and harness validation.

## Fix-Agent Output

State transition: `reviewing -> fixing`.

Fixed findings:

- `create_match` and `join_match` are now accepted only in the `Authenticated` phase.
- `auth_request` and `handshake` are now accepted only in the `Connected` phase.
- Added negative tests for create-after-join and repeated-auth ordering.

Changed files:

- `src/battle_protocol/Protocol.cpp`
- `tests/unit/protocol/ProtocolEnvelopeTests.cpp`
- `docs/agent-runs/0021-protocol-playable-game-messages.md`

Remaining risks:

- Backend session/player ownership validation remains a later task.

Ready for re-verification: yes.

## Verification-Agent After Fixes

State transition: `fixing -> re-verifying`.

Commands/results:

- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- `git diff --check` - passed, with line-ending warnings only.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `C:\Program Files\Git\bin\bash.exe -lc 'python3() { "/c/Users/user/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe" "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'` - passed.

Verification decision: approve.

State transition: `re-verifying -> commit-ready`.

## Commit-Agent Output

State transition: `commit-ready -> committed`.

Commit policy:

- One scoped task commit.
- No forbidden files.
- No generated dependency directories or build outputs.
- Secret scan passed before commit.

Commit:

- `protocol: add playable message validation`
- Final amended commit hash is recorded in the Coordinator closeout response.

## Coordinator Closeout

Final status: committed.

Known risks:

- Backend session/player ownership validation remains a later task.
- Frontend/runtime validators are deferred to the Telegram/WebSocket slice.

Follow-up:

- Merge task `0021` into `master` after post-commit sanity checks, then start task `0022-backend-match-loop-and-fake-sessions.md`.
