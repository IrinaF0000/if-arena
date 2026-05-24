# Task 0039: Server tick, snapshot and session lifecycle audit

## Goal

Make the authoritative backend loop explicit and reliable: ticks advance independently of client input, snapshots are emitted at configured `snapshotRate`, passive sessions stay alive through keepalive semantics, and match completion is visible through server snapshots/events.

## Scope

Allowed files:

- `src/battle_backend/MatchLoop.hpp`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_backend/README.md`
- `src/battle_server_app/main.cpp`
- `src/battle_server_app/README.md`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/integration/server/tcp_live_tick_smoke.py`
- `tests/frontend/websocket_local_smoke.py`
- `docs/agent-runs/0039-server-tick-session-lifecycle-audit.md`
- `docs/agent-tasks/0039-server-tick-session-lifecycle-audit.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `src/battle_core/**` unless a blocking audit finding proves the core contract is wrong
- `frontend/**` except the existing WebSocket smoke listed above
- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- reconnect/resume implementation from task 0040
- release tags
- pushing to GitHub

## Required behavior

- Preserve server-authoritative ticks independent of client input.
- Respect configured `snapshotRate` for periodic snapshot fanout while keeping `tickRate` as the simulation tick rate.
- Keep immediate post-join snapshots so newly matched clients see the first state promptly.
- Keep passive active-match sessions connected by ping/pong semantics while snapshots continue.
- Keep match-over visibility through existing `finished` snapshots and `match_finished` event batches.
- Do not move gameplay truth into transports or clients.

## Required quality gates

- Gate A. Every implementation task
- Gate F. Security-sensitive network/session lifecycle changes
- Gate I. Performance/load-sensitive tick and broadcast behavior
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/server/tcp_live_tick_smoke.py`
- `python tests/frontend/websocket_local_smoke.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Security-Agent: yes
- Architecture-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Server tick progression no longer depends on fresh client input after match start.
- Snapshot broadcast cadence follows `snapshotRate` instead of every simulation tick.
- Existing long idle TCP and WebSocket smoke scenarios still pass.
- Tests cover late input after long runtime and passive snapshot receipt.
- No reconnect/resume behavior from task 0040 is introduced.

## Rollback note

Revert this task commit to restore previous every-tick snapshot fanout and remove the related test/doc updates.
