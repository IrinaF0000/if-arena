# Backend Security Utilities

This directory is reserved for backend-side security helpers.

Planned components:

- safe logging and redaction helpers;
- session token validation helpers;
- Telegram auth integration helpers;
- constant-time comparison utilities where appropriate;
- input validation helpers shared by transports.

Rules:

- do not place real secrets here;
- do not hardcode tokens, signing keys, or certificates;
- do not log raw auth payloads;
- keep helpers transport-neutral where possible.

Current helpers:

- `TelegramAuthValidator` validates raw Mini App `initData` with config-provided bot token, HMAC-SHA256 hash check, freshness check, size limit, and fail-closed errors.
- `redactTelegramInitDataForLog()` returns generic redaction text and must be used instead of logging raw init data.
- `redactForLog()` is the current centralized safe-logging placeholder and returns generic redaction text for any non-empty sensitive value.
