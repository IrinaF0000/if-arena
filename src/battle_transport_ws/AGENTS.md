# WebSocket Transport Agent Rules

WebSocket transport exposes the Telegram Mini App endpoint.

## Required behavior

- Use WSS in public deployments.
- Apply the same transport-neutral protocol messages as TCP.
- Enforce message size limits and rate limits.
- Authenticate Telegram sessions before allowing gameplay commands.
- Do not trust frontend-reported Telegram identity.
- Do not log tokens, raw initData, session secrets, or full auth headers.

## Required tests

Add tests for malformed frames/messages, unauthenticated command attempts, expired auth data, replayed auth data, oversized messages, and rate-limit violations.

Read also:

- `.agents/skills/websocket-gateway/SKILL.md`
- `.agents/skills/telegram-miniapp-security/SKILL.md`
- `docs/agent-rules/security/TELEGRAM_AUTH.md`
