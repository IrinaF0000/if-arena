# Telegram Auth Security

Telegram Mini App identity must be validated on the backend.

## Required

- Do not trust `initDataUnsafe`.
- Send raw `initData` to backend for validation.
- Validate hash/HMAC according to Telegram Mini Apps documentation.
- Check `auth_date` freshness.
- Bind backend session token to validated Telegram user id and connection/session.
- Expire session tokens.
- Use WSS in public deployments.
- Never log bot token, raw initData, auth headers, session secrets, or full signed payloads.

## Forbidden

- Frontend-only auth.
- Client-reported Telegram user id as authoritative identity.
- Long-lived unaudited tokens.
- Committing bot token or production secrets.
