# ADR 0010: Backend-Validated Telegram Mini App Auth

## Status

Accepted.

## Context

Telegram Mini App provides initialization data to the frontend. Frontend data is not trusted. Telegram documentation requires validating raw init data before trusting it on the server side.

## Decision

The Mini App sends raw `initData` to the backend in `auth_request`. The backend validates it and maps the Telegram user to an internal player identity.

## Consequences

Positive:

- frontend cannot spoof identity by editing JavaScript state;
- bot token stays backend-only;
- session identity is controlled by backend.

Negative:

- backend needs Telegram validation component;
- tests require fixtures;
- public deployment needs secure bot token configuration.

## Rules

- Never trust `initDataUnsafe`.
- Never put bot token in frontend.
- Never log raw `initData`.
- Check freshness of auth data.
