# Agent Tasks

Task packets define task-specific scope. Common rules are canonical in:

- `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`
- `docs/review/QUALITY_GATES.md`
- `docs/agent-rules/quality/TESTING.md`

Rules:

- One task per run; do not merge tasks or broaden scope.
- Never stage build outputs, generated dependency directories, or real secret files.
- Complete required gates, reviews, checks, and Test Impact Matrix before commit.
- For changed behavior, test happy path, corner cases, invalid/hostile input, authority violations, and resource bounds where relevant.

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
13. `0016-clean-foundation-before-playable-slice.md`
14. `0017-core-arena-map-and-config.md`
15. `0018-core-player-movement-and-collision.md`
16. `0019-core-objective-run-rules.md`
17. `0020-core-combat-dash-and-hazards.md`
18. `0021-protocol-playable-game-messages.md`
19. `0022-backend-match-loop-and-fake-sessions.md`
20. `0023-server-app-and-local-config.md`
21. `0024-cli-two-player-playable-flow.md`
22. `0025-tcp-vertical-slice-integration.md`
23. `0026-websocket-and-telegram-playable-slice.md`
24. `0027-qt-playable-client.md`
25. `0028-load-security-hardening-and-portfolio.md`
26. `0030-release-stabilization-and-qt-verification.md`
27. `0031-fix-playable-controls-and-arena-readability.md`

Start multi-task Codex orchestration with `docs/agent-seeds/CODEX_ORCHESTRATOR_PROMPT.md`.
