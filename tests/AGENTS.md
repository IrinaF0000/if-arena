# Test Agent Rules

Tests are mandatory for protocol, security, backend, and gameplay changes.

## Rules

- Add negative tests for untrusted input boundaries.
- Add deterministic gameplay tests for `battle_core` changes.
- Add load or stress tests for scalability changes.
- Avoid flaky sleeps. Prefer deterministic clocks or test schedulers where possible.
- Tests should not require real Telegram, real cloud, or production secrets.
