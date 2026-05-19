# ADR 0011: Strict TypeScript for Telegram Mini App

## Status

Accepted.

## Context

The project should demonstrate high quality in all used languages, not just C++.

## Decision

Telegram Mini App is written in strict TypeScript with ESLint and Prettier. Runtime validation is required for inbound WebSocket messages.

## Consequences

Positive:

- safer frontend code;
- easier agent review;
- clearer protocol integration.

Negative:

- more setup and checks;
- frontend tasks require frontend-specific quality gates.

## Rules

- No unchecked `any`.
- Treat network input as `unknown`.
- No frontend secrets.
- No authoritative game rules in frontend.
