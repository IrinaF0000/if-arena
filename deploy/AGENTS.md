# Deployment Agent Rules

Deployment files are protected because they can affect public exposure and secrets.

## Rules

- Do not edit deployment files unless explicitly allowed by the task packet.
- Never commit real secrets, Telegram bot tokens, TLS private keys, cloud credentials, or production URLs requiring secrecy.
- Public deployment must use WSS for Telegram Mini App traffic.
- Expose only required ports.
- Document firewall, environment variables, and rollback steps.
- Keep demo deployment small, bounded, and cost-controlled.

Read also: `docs/agent-rules/security/DEPLOYMENT.md`.

## Secret boundaries

- Use `.env.example` and provider-specific documentation for required variables.
- Never commit real `.env` files, TLS private keys, cloud credentials, or Telegram bot tokens.
- Deployment examples may name environment variables, but must not contain their values.
- Any deployment change touching secret loading requires Security Review Agent approval.
