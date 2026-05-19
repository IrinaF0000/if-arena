# Agent Tasks

This directory contains ready-to-use task packets. The Coordinator should copy one task into `AGENT_PROGRESS.md`, adjust exact paths if needed, and assign it to one agent.

Rules:

- One task per agent run.
- Do not merge multiple task packets.
- Do not broaden scope.
- Follow `docs/agent-seeds/LLM_TOKEN_BUDGET.md`.
- Complete required quality gates before marking done.

Recommended order:

1. `0001-bootstrap-build.md`
2. `0002-import-old-project-snapshot.md`
3. `0003-core-extraction-audit.md`
4. `0004-battle-core-facade.md`
5. `0005-protocol-envelope-and-limits.md`
6. `0006-backend-session-model.md`
7. `0007-tcp-transport-framing.md`
8. `0008-qt-client-network-skeleton.md`
9. `0009-websocket-transport-skeleton.md`
10. `0010-telegram-mini-app-skeleton.md`
11. `0011-telegram-auth-validation.md`
12. `0012-load-client-foundation.md`
