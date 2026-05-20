# Task 0028: Harden, load test, and polish portfolio docs

## Task title
`Harden, load test, and polish portfolio docs`

## Phase
`Phase 9/10/12 - hardening and portfolio`

## Goal
Make the playable project reviewable: security hardening, reproducible load report, demo instructions, honest limitations, screenshots/GIF plan, and CV-ready technical summary.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `tools/load_client/**`
- `reports/load/**`
- `tests/load/**`
- `tests/security/**`
- `docs/security/**`
- `docs/performance/**`
- `docs/operations/**`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `README.md`
- `replays/**`
- `scripts/** only if non-CI helper script is required`
- `docs/agent-runs/`

## Forbidden files
- `.github/workflows/** unless separate CI task`
- `deploy/** unless only docs are explicitly allowed`
- `src/battle_core/** except bug fixes from hardening tests`
- `src/battle_qt_client/** except docs/screenshots`
- `frontend/** except docs/screenshots`
- `external/**`

## Required quality gates
- Gate A
- Gate F
- Gate G
- Gate H
- Gate J if auth/logging touched
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Add or complete load scenarios: normal clients, slow readers, command spam, invalid payloads.
2. Record reproducible local load report with environment, limits, and honest numbers.
3. Complete threat model updates for implemented transports and clients.
4. Add negative tests for security scenarios not yet covered.
5. Add README quickstart for server, CLI, Telegram, and Qt where implemented.
6. Add known limitations.
7. Add architecture and gameplay demo notes.
8. Add CV/LinkedIn bullet suggestions if desired.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- Scenario C is covered by tests and docs.
- Load report is reproducible and not exaggerated.
- README matches actual code behavior.
- Security notes mention remaining risks honestly.
- No deployment secrets or generated assets are committed accidentally.

## Required reviews
- Coordinator: yes
- Architecture-Agent: no unless APIs change
- Security-Agent: yes
- Performance-Agent: yes
- Frontend-Agent: yes if frontend docs/assets change
- Qt-Agent: yes if Qt docs/assets change
- Verification-Agent: yes

## Rollback note
One scoped commit.
