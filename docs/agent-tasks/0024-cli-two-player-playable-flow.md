# Task 0024: Add CLI two-player playable flow

## Task title
`Add CLI two-player playable flow`

## Phase
`Phase 5 - CLI and load clients`

## Goal
Make `battle_cli_client` useful for debugging: connect or fake-connect, create/join a match, send scripted intentions, and print snapshots/events.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_cli_client/**`
- `src/battle_protocol/** only if CLI integration reveals missing helpers`
- `tests/integration/**`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `README.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_core/** except via public API`
- `frontend/**`
- `src/battle_qt_client/**`
- `deploy/**`
- `external/**`

## Required quality gates
- Gate A
- Gate B if protocol helpers change
- Gate C if command semantics are tested through CLI
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add CLI args for endpoint, auth mode, create/join, match code, script file, and interactive mode if practical.
2. Add scripted command input for Scenario B.
3. Print snapshots and events in a readable format.
4. Support a fake/in-process mode if real TCP is not ready, but label it clearly.
5. Add integration smoke test where feasible.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- A developer can use CLI to drive a local playable match path.
- CLI sends intentions, not authoritative state.
- README documents exact commands.

## Required reviews
- Coordinator: yes
- Architecture-Agent: no unless public APIs change
- Security-Agent: yes if TCP/network is used
- Performance-Agent: no
- Verification-Agent: yes

## Rollback note
One scoped commit.
