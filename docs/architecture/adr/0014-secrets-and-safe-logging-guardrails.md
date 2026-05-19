# ADR 0014: Secrets and Safe Logging Guardrails

## Status

Accepted

## Context

The project is intended to be public portfolio code, but it includes Telegram Mini App authentication, public deployment examples, TLS configuration, and backend sessions. These areas often require secrets. Accidental commits or unsafe logs would be high-impact for both security and professionalism.

## Decision

The repository keeps only placeholder secret examples. Real values must be supplied through environment variables, GitHub Secrets, or cloud secret stores. PR CI and main CI run a lightweight secret scanner. Backend logging must use centralized redaction helpers before handling sensitive auth or transport data.

## Consequences

- `.env.example` files document required variables without values.
- `.gitignore` excludes real secret files, certificates, keys, and local config directories.
- CI rejects obvious token/key leaks.
- Auth, logging, deployment, and CI secret changes require Security Review Agent approval.
- The custom scanner is a guardrail, not a replacement for provider-side secret scanning or careful review.
