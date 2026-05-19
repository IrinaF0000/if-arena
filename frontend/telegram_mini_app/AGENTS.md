# Telegram Mini App Agent Rules

This frontend is a strict TypeScript Telegram Mini App that renders the arena and communicates with the backend through WebSocket.

## Required standards

- Strict TypeScript.
- No `any` unless justified in the task report.
- No authoritative game rules in frontend.
- No trust in `initDataUnsafe`.
- Send raw `initData` only to backend auth endpoint/channel as designed.
- Use WSS in public config.
- Keep touch controls accessible and responsive.
- Avoid excessive dependencies.

## Required checks

```bash
npm run typecheck
npm run lint
npm run build
```

Read also:

- `.agents/skills/telegram-miniapp-security/SKILL.md`
- `docs/agent-rules/quality/TYPESCRIPT.md`
- `docs/frontend/TYPESCRIPT_FRONTEND_STANDARDS.md`
