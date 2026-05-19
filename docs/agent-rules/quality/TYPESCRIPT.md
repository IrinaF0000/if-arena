# TypeScript Quality Rules

## Required

- Strict TypeScript.
- No implicit `any`.
- Avoid `any`; if unavoidable, justify it in the task report.
- Runtime validation for backend messages before rendering.
- Keep Telegram integration, network client, game rendering, and UI controls separated.
- Use small modules with explicit interfaces.
- No authoritative game rules in frontend.

## Checks

Run these for frontend work:

```bash
npm run typecheck
npm run lint
npm run build
```
