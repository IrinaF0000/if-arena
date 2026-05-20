# Task 0022: Backend Match Loop And Fake Sessions

## Status

Implemented on branch `agent/0022-backend-match-loop-and-fake-sessions`.

## Changed files

- `src/battle_backend/MatchLoop.hpp`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/Session.hpp`
- `src/battle_backend/README.md`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `docs/architecture/TECHNICAL_ARCHITECTURE.md`
- `docs/agent-runs/0022-backend-match-loop-and-fake-sessions.md`

## Implementation notes

- Added `MatchManager` with deterministic join-code flow, backend-owned session/player ownership, two-player match startup, and direct `tick` driver for in-process tests.
- Added bounded command queues, per-session pending command limits, per-tick rate limits, monotonic session sequence validation, and claimed-player mismatch rejection.
- Added snapshot/event broadcast into existing bounded outbound queues. Slow sessions close via `QueueOverflow`.
- Added metrics for match creation/activity, accepted/applied/rejected commands, snapshot/event broadcasts, disconnects, and queue overflows.

## Test matrix

- Backend unit/in-process tests cover match creation/join, command ownership, sequence rejection, wrong session rejection, spam rate limiting, bounded outbound behavior, and fake-session snapshot/event delivery.
- Backend in-process coverage includes a full Objective Run match on the canonical arena: fake sessions move around center obstacles, pick up the objective, return to base, score, and finish the match without sockets.
- Full project checks were run after fixes.

## Checks run

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh` via Git Bash with the bundled Python shim
- `git diff --check`

## Risks and follow-up

- Payloads are minimal backend JSON strings for local/in-process routing. Transport-level protocol envelope integration remains for later packets.
- The playable backend slice intentionally supports exactly two players for Objective Run.
