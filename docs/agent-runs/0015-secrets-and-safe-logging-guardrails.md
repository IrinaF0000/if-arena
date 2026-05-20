# 0015 Secrets and Safe Logging Guardrails

Date: 2026-05-19

## Summary

- Verified `.env.example` and `frontend/telegram_mini_app/.env.example` use placeholders only.
- Verified `.gitignore` excludes real `.env` and key/credential files while allowing examples.
- Narrowed `.gitleaks.toml` so Markdown and docs are not broadly allowlisted.
- Documented the existing `redactForLog()` safe-logging placeholder.

## Changed files

- `.gitleaks.toml` - removed broad docs/Markdown path allowlist entries.
- `src/battle_backend/security/README.md` - documented `redactForLog()`.
- `docs/agent-runs/0015-secrets-and-safe-logging-guardrails.md` - recorded task progress.

## Checks run

- `python scripts\ci\scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed using Git Bash with a local `python3` shim.
- `gitleaks version` - not available; `gitleaks` is not installed in the local environment.

## Risks and follow-up

- Security Review Agent approval is required before merge.
- CI secret scanning remains enabled in the reviewed PR and main workflows.
