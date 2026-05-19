# Task 0015: Maintain secrets and safe logging guardrails

## Goal

Keep the repository safe to publish by enforcing placeholder-only secrets, safe logging, and CI secret scanning.

## Allowed files

- `.env.example`
- `frontend/telegram_mini_app/.env.example`
- `.gitignore`
- `.gitleaks.toml`
- `scripts/ci/scan_secrets.py`
- `docs/security/SECRETS_MANAGEMENT.md`
- `docs/agent-rules/security/SECRETS_AND_LOGGING.md`
- `docs/review/QUALITY_GATES.md`
- `docs/review/CODE_REVIEW_CHECKLIST.md`
- `src/battle_backend/security/**`

## Forbidden files

- real `.env` files;
- any file containing real Telegram tokens, TLS private keys, cloud credentials, or session secrets;
- unrelated gameplay, transport, frontend, or CI workflow files unless a dedicated task allows them.

## Required quality gates

- Gate A: every implementation task.
- Gate F: security-sensitive changes.
- Gate J: secrets, auth, and logging changes.

## Required checks

```bash
python3 scripts/ci/scan_secrets.py
bash scripts/ci/validate_structure.sh
```

## Acceptance criteria

- `.env.example` contains placeholders only.
- Frontend `.env.example` contains no backend secrets or Telegram bot token.
- `.gitignore` excludes real secret files while allowing examples.
- CI secret scanning remains enabled.
- Safe logging/redaction placeholder exists for future backend logging.
- Security Review Agent approval is required for future changes in this area.
