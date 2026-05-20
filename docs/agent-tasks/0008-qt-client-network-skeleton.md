# Task: Create Qt client network skeleton

## Phase

Phase 6. Qt desktop client

## Goal

Create a Qt NetworkClient skeleton separated from widgets.

## Allowed files

- `src/battle_qt_client/`
- `docs/game/CONTROLS_AND_UI.md`
- `tests/manual/`

## Forbidden files

- `src/battle_core/`
- `src/battle_backend/`

## Required quality gates

- Gate A
- Gate D

## Security impact

medium

## Performance/scalability impact

medium

## Architecture impact

medium

## Token budget instructions

- Read only Qt client docs/files and protocol message overview.

## Implementation steps

1. Add NetworkClient class skeleton.
2. Add connection state enum.
3. Add signal/slot plan.
4. Keep UI placeholder simple.
5. Add manual checklist for connect/disconnect.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- Build target if Qt option enabled in environment.
- Manual checklist updated.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Qt-Agent: yes
- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope remains narrow.
- Checks pass or blockers are documented.
- Run note is updated.
