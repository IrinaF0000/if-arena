# Skill: Telegram Mini App Security

Use for Telegram Mini App frontend, backend Telegram auth, or session identity.

## Checklist

- Do not trust `initDataUnsafe`.
- Backend validates raw `initData`.
- Check auth freshness.
- Bind session token to validated Telegram user.
- Use WSS in public config.
- Do not log auth payloads or tokens.
- Frontend contains no authoritative game rules.
