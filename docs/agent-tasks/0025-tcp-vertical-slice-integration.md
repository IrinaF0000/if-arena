# Task 0025: Complete TCP vertical slice integration

## Task title
`Complete TCP vertical slice integration`

## Phase
`Phase 4/5 - raw TCP playable slice`

## Goal
Connect the raw TCP transport to backend sessions so two CLI clients can play a real local match over TCP.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_transport_tcp/**`
- `src/battle_backend/**`
- `src/battle_server_app/**`
- `src/battle_cli_client/**`
- `tests/unit/transport/**`
- `tests/integration/**`
- `docs/architecture/TRANSPORT_ABSTRACTION.md`
- `docs/architecture/PROTOCOL.md`
- `README.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_core/** unless public facade bug blocks integration`
- `frontend/**`
- `src/battle_qt_client/**`
- `deploy/**`
- `external/**`

## Required quality gates
- Gate A
- Gate B
- Gate C
- Gate F
- Gate G
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Implement or complete TCP accept/read/write path.
2. Use existing length-prefixed frame codec with partial and combined frame handling.
3. Reject oversized frames before large allocation.
4. Map connection lifecycle to backend session lifecycle.
5. Handle disconnect, idle timeout, ping/pong, slow writer, and bounded outgoing queue.
6. Add integration test with two local TCP clients if test infrastructure allows.
7. Document local run and play commands.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Scenario B passes over real TCP.
- Scenario C hostile TCP cases pass.
- No TCP code leaks into battle_core.
- Slow clients and spam are bounded and tested.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
