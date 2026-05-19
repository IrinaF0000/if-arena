# Task: Create BattleEngine facade

## Phase

Phase 1. Original project import and core extraction

## Goal

Introduce a minimal `BattleEngine` facade in `battle_core` for server use.

## Allowed files

- `src/battle_core/`
- `tests/unit/`
- `docs/architecture/TECHNICAL_ARCHITECTURE.md` if public API differs

## Forbidden files

- `src/battle_transport_tcp/`
- `src/battle_transport_ws/`
- `src/battle_qt_client/`
- `frontend/`

## Required quality gates

- Gate A
- Gate C

## Security impact

low

## Performance/scalability impact

low

## Architecture impact

high

## Token budget instructions

- Read only extraction map and battle_core files.
- Do not inspect transport/client modules.

## Implementation steps

1. Add minimal facade types for match config, input command, snapshot, event, command result.
2. Keep implementation placeholder or adapt extracted core incrementally.
3. Add deterministic unit test for facade behavior.
4. Ensure no network/UI dependency appears.

## Required tests/checks

- Unit test for creating a match/facade object.
- Dependency check by Architecture-Agent.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope is not broadened.
- Required checks pass or blockers are documented.
- Agent progress is updated with files changed, tests run, and remaining risks.
