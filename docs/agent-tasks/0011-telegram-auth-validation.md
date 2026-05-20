# Task: Implement Telegram auth validation

## Phase

Phase 8/9. Telegram Mini App and Security hardening

## Goal

Validate Telegram Mini App raw initData on backend.

## Allowed files

- `src/battle_backend/`
- `src/battle_transport_ws/` if auth is wired there
- `tests/security/`
- `docs/telegram/TELEGRAM_SECURITY.md`
- `docs/security/SECURITY_REQUIREMENTS.md`

## Forbidden files

- `frontend/telegram_mini_app/` except auth request shape if needed
- Any secret files

## Required quality gates

- Gate A
- Gate F
- Gate B if protocol changes

## Security impact

high

## Performance/scalability impact

medium

## Architecture impact

high

## Token budget instructions

- Read only Telegram security docs and backend auth files.
- Do not paste real bot token or raw personal initData.

## Implementation steps

1. Add TelegramAuth validator component.
2. Accept bot token from secure config/env only.
3. Validate hash/signature.
4. Check auth_date freshness.
5. Redact initData in logs.
6. Add negative tests.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- Valid fixture passes.
- Invalid hash fails.
- Stale auth_date fails.
- Oversized initData fails.
- Raw initData is not logged.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Security-Agent: yes
- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope remains narrow.
- Checks pass or blockers are documented.
- Run note is updated.
