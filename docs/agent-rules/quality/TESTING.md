# Testing Rules

## Required test categories

- Unit tests for protocol, rate limits, validation, deterministic gameplay.
- Integration tests for server sessions and match flow.
- Negative tests for malformed input and unauthorized commands.
- Load tests for simulated clients, slow clients, and spam clients.
- Manual checklist for Qt UI until automated UI tests exist.

## Test design

- Prefer deterministic clocks and seeds.
- Avoid flaky sleeps.
- Do not require real Telegram, real cloud services, or production secrets.
- Tests must fail closed for invalid input.
