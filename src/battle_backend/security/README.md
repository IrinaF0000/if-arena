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
