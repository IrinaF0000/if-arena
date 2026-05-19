# ADR 0008: One Backend with Qt and Telegram Mini App Clients

## Status

Accepted.

## Context

The project should demonstrate both C++/Qt desktop UI and a Telegram Mini App UX. The game logic and match authority must not be duplicated.

## Decision

Use one authoritative backend process with two transport adapters:

- raw TCP for Qt/CLI/load clients;
- WebSocket for Telegram Mini App.

Both transports feed the same backend session/match system and the same protocol message model.

## Consequences

Positive:

- one source of truth for game rules;
- shared load/security/metrics infrastructure;
- easier to compare TCP and WebSocket behavior;
- better portfolio story.

Negative:

- more architecture documentation required;
- WebSocket and Telegram auth add security requirements;
- frontend TypeScript quality gates are needed.

## Rules

- Clients do not link to `battle_core`.
- Transports do not implement game rules.
- Backend owns all authority.
