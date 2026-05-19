# battle_protocol Agent Rules

`battle_protocol` contains transport-independent message DTOs, validation helpers, limits, and encoding rules shared by TCP and WebSocket.

## Responsibilities

- Protocol envelope and versioning.
- Message kinds and payload schemas.
- Size limits and validation constraints.
- Transport-neutral error types.

## Rules

- Keep protocol objects free of Qt, socket, and browser dependencies.
- Version protocol changes explicitly.
- Update `docs/architecture/PROTOCOL.md` for schema changes.
- Add backward/invalid input tests for every protocol change.
- Reject unknown or malformed client messages safely.

Read also: `docs/agent-rules/security/NETWORK_INPUT.md`.
