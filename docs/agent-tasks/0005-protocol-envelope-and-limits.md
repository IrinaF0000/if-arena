# Task: Implement protocol envelope and limits

## Phase

Phase 2. Protocol foundation

## Goal

Add transport-independent protocol envelope, limits, and validation helpers.

## Allowed files

- `src/battle_protocol/`
- `tests/unit/protocol/`
- `docs/architecture/PROTOCOL.md`

## Forbidden files

- `src/battle_core/`
- `src/battle_qt_client/`
- `frontend/` unless adding generated schema is explicitly included

## Required quality gates

- Gate A
- Gate B
- Gate F

## Security impact

high

## Performance/scalability impact

medium

## Architecture impact

medium

## Token budget instructions

- Read protocol docs and battle_protocol only.
- Do not read clients unless absolutely necessary.

## Implementation steps

1. Define envelope type.
2. Define message type enum/string constants.
3. Add protocol limit constants/config.
4. Add parser that returns error result, not uncaught exception.
5. Add serializer.
6. Add negative tests.

## Required tests/checks

- Valid envelope parses.
- Malformed JSON rejected.
- Unknown type rejected.
- Oversized string rejected.
- Missing fields rejected.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Security-Agent: yes
- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope is not broadened.
- Required checks pass or blockers are documented.
- Agent progress is updated with files changed, tests run, and remaining risks.
