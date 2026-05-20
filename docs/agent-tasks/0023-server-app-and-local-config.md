# Task 0023: Make server app runnable

## Task title
`Make server app runnable`

## Phase
`Phase 3/4 - server app`

## Goal
Turn `battle_server_app` into a real local process that loads config, initializes backend, starts enabled transports, logs metrics, and shuts down cleanly.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `src/battle_server_app/**`
- `src/battle_backend/**`
- `src/battle_transport_tcp/** only for construction interface`
- `src/battle_transport_ws/** only for construction interface`
- `config/examples/**`
- `scripts/run_local_server.sh`
- `scripts/run_public_server.sh`
- `docs/deployment/**`
- `README.md`
- `docs/agent-runs/`

## Forbidden files
- `src/battle_core/** unless public API bug blocks integration`
- `src/battle_qt_client/**`
- `frontend/**`
- `.github/workflows/**`
- `external/**`

## Required quality gates
- Gate A
- Gate F
- Gate G
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add command-line args for config path and local mode.
2. Load `config/examples/server.local.json`.
3. Initialize backend limits from config.
4. Start TCP and/or WebSocket transports when implemented, otherwise fail with clear message.
5. Add graceful shutdown path.
6. Log safe startup, metrics, and shutdown information without secrets.
7. Fix run scripts to use the correct binary and config.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- `./build/battle_server_app --config config/examples/server.local.json` starts or fails with actionable error if transport is not implemented yet.
- Server app no longer silently exits with success while doing nothing.
- No raw secrets or auth payloads are logged.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit.
