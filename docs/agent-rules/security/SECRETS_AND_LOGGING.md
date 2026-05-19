# Secrets and Logging

All agents must treat this repository as publishable by default. Real secrets, private credentials, and sensitive auth payloads must never be committed or printed in logs.

## Never commit

- Telegram bot tokens;
- backend session signing keys;
- TLS private keys or real certificates;
- cloud credentials;
- SSH private keys;
- personal access tokens;
- production session secrets;
- private IPs or URLs that must remain private;
- `.env` files with real values.

Only placeholder examples are allowed, for example `.env.example` and `config/examples/*.json`.

## Secret storage

Use environment variables, GitHub Secrets, cloud secret stores, or local ignored `.env` files. Config files may reference environment variable names, but must not contain secret values.

## Logging rules

Allowed:

- structured event names;
- sanitized session id or connection id;
- error category;
- transport type;
- counters and timing metrics.

Forbidden by default:

- raw Telegram `initData`;
- `initDataUnsafe`;
- Telegram bot token;
- backend session tokens;
- `Authorization` headers;
- raw TCP frames;
- raw WebSocket payloads;
- TLS private key paths if they reveal private infrastructure.

Prefer messages such as:

```text
telegram_auth_failed reason=invalid_signature user_id_present=true
frame_rejected reason=oversized transport=tcp
command_rejected reason=rate_limit session_id=s_123
```

Do not log messages such as:

```text
raw initData=...
Authorization: Bearer ...
rawFrame=...
```

## Redaction requirement

Any backend logging API must provide a central redaction helper before production-like logging is enabled. Do not scatter ad-hoc redaction across call sites.

## Required checks

- Run `python3 scripts/ci/scan_secrets.py` before opening a PR.
- Any changes touching auth, logging, environment loading, deployment, or CI secrets require Security Review Agent approval.
