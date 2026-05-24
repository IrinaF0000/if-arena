# Task 0039: Server tick, snapshot and session lifecycle audit

## State

committed

## Coordinator Preflight

- Branch: `agent/0039-server-tick-session-lifecycle-audit`.
- Baseline: `master` after local merge of task 0038.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0039-server-tick-session-lifecycle-audit.md`.
- Allowed implementation scope confirmed: backend match loop API/implementation, server app runtime loop/docs, focused TCP/WebSocket lifecycle tests, task docs.
- Forbidden scope confirmed: no `battle_core` change unless blocking, no reconnect/resume, no workflows, deploy files, release tags, push, or frontend app work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0039. Audit and minimally fix the authoritative server loop so simulation ticks continue independently of client input and periodic snapshot fanout follows configured `snapshotRate`. Preserve immediate post-join snapshots and existing passive keepalive semantics. Add/update focused tests for snapshot cadence, passive runtime, late input, and match-over visibility where touched. Do not implement reconnect/resume or change gameplay rules in `battle_core`.

## Implementation-Agent

- Added `MatchManager::tick(match, broadcastSnapshot)` with a default of `true` so existing tests and direct backend callers preserve prior behavior unless the server runtime explicitly throttles snapshot fanout.
- Kept authoritative simulation ticks independent of client input: tick still drains queued commands, advances `BattleEngine`, emits event batches, resets per-tick command counters, and updates queryable match views even when snapshots are not broadcast.
- Added per-active-match snapshot schedule to TCP and WebSocket server runtime state. Matches still tick at `server.tickRate`, while periodic snapshots are broadcast when `server.snapshotRate` is due.
- Preserved immediate post-join snapshots by keeping the existing join-time tick with snapshot fanout.
- Replaced the backend Objective Run max tick magic number with a named constant and documented that `match_finished` events plus `finished` snapshots expose the duration cap.
- Updated backend/server README notes for tick/snapshot cadence.

## Test Impact Matrix

Changed behavior:

- TCP and WebSocket server loops now throttle periodic snapshot fanout to configured `server.snapshotRate` instead of broadcasting a snapshot on every simulation tick.
- `MatchManager::tick` can advance authoritative state without broadcasting a snapshot, while retaining the default snapshot fanout behavior for existing backend tests/callers.

Tests added/updated:

- Happy path: `battle_backend_tests` covers normal command queue, tick, event, and snapshot broadcast behavior.
- Corner cases: new backend unit test proves simulation tick advances and match view remains queryable when snapshot fanout is suppressed until due.
- Invalid input / hostile input: existing protocol/session validation tests and TCP/WebSocket smoke tests remain passing; no parser/auth behavior was loosened.
- Authority / ownership: existing backend ownership/sequence/wrong-session tests remain passing; clients still send intentions only.
- Resource bounds / performance: TCP live tick smoke now proves `snapshotRate` does not broadcast on every simulation tick, while long passive runtime and late input still work after tick 1201.
- Regression: `tcp_live_tick_smoke.py` still proves ticks continue beyond the prior cap and late input applies after long idle runtime; `websocket_local_smoke.py` still proves passive WebSocket clients stay connected and receive snapshots beyond idle timeout.
- Manual UI checks: none required; this task changes backend/server cadence and is covered by automated smoke/unit tests.

Not tested and why:

- Reconnect/resume behavior is task 0040 and intentionally not implemented or tested here.
- Additional Playwright browser E2E was requested as a bonus WebSocket check, but sandbox escalation was rejected due the session usage limit. The required WebSocket protocol smoke passed locally.
- GitHub-hosted CI was not run because no push was performed.

Ready for verification: yes.

## Verification-Agent

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 7/7 tests.
- `python tests/integration/server/tcp_live_tick_smoke.py` - passed.
- `python tests/frontend/websocket_local_smoke.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed.
- `scripts/ci/validate_structure.sh` through Git Bash shim - passed.

## Review-Agent

- Security Review-Agent: approved. Network/auth parsing is unchanged; existing hostile input, handshake, idle timeout, and bounded queue checks remain active.
- Architecture Review-Agent: approved. Simulation truth remains in `battle_core`; server app only schedules backend ticks/snapshots; transports do not own gameplay state.
- Performance Review-Agent: approved. Snapshot fanout is now rate-limited by `snapshotRate`, while catch-up remains bounded to avoid long lock-held bursts.
- Verification-Agent: approved. Required local checks passed; optional Playwright E2E could not run because sandbox approval was rejected.

## Fix-Agent

- No review-blocking fixes required after verification.

## Commit-Agent

- Commit created with message `server: honor snapshot rate in lifecycle loop`.
- Final commit hash is recorded in the coordinator response because amending this file changes the hash.
- Scope checked before commit: only task 0039 files were staged; root roadmap file remained unstaged.

## Coordinator Closeout

- Task 0039 is complete on branch `agent/0039-server-tick-session-lifecycle-audit`.
- Final commit hash is recorded in the coordinator response.
- Ready for local merge to `master`.
- No push or release tag was performed.
