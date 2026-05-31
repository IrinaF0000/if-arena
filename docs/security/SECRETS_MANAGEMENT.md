# Secrets Management

This project must be safe to publish as a public public repository. All real secrets stay outside git.

## Secret locations

Use these locations for real values:

- local development: `.env` or OS environment variables;
- GitHub Actions: GitHub Secrets or Environment secrets;
- public deployment: the cloud provider secret store or environment variable injection;
- local TLS tests: files outside the repository or ignored by `.gitignore`.

## Values that must never be committed

- Telegram bot token;
- session signing keys;
- TLS private keys and certificates used by a real deployment;
- cloud credentials and SSH private keys;
- personal access tokens;
- production database URLs, if any are added later;
- private service URLs or IPs that are not intended to be public.

## Required repository files

- `.env.example` may contain only placeholder values.
- `frontend/telegram_mini_app/.env.example` must contain only public frontend values.
- `config/examples/*.json` may refer to environment variable names, never secret values.
- `.gitignore` must ignore local secret files.
- `scripts/ci/scan_secrets.py` must run in PR CI and main CI.

## Telegram-specific rules

- The Telegram bot token exists only on the backend side.
- The Mini App frontend must never receive or embed the bot token.
- The frontend may send Telegram `initData` to the backend.
- The backend validates `initData`, then creates a short-lived backend session.
- Do not log raw `initData`, backend session tokens, auth headers, or signed payloads.

## Rotation rule

If a real secret is committed, assume it is compromised:

1. Remove it from the repository.
2. Rotate/revoke it immediately.
3. Check logs and CI artifacts for exposure.
4. Add a regression test or secret-scanning rule if the leak pattern was not detected.

## Agent rule

Any task that changes auth, logging, deployment, CI secret handling, or environment-variable loading requires Security Review Agent approval.
