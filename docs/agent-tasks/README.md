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
28. `0032-fix-websocket-client-envelope-and-idle-timeout.md`
29. `0033-fix-core-movement-mechanics-and-arena-rules.md`
30. `0034-document-and-enforce-architecture-boundaries.md`
31. `0036-integrate-svg-player-assets.md`
32. `0037-github-ci-and-public-release-baseline.md`
33. `0038-browser-e2e-playwright-harness.md`
34. `0039-server-tick-session-lifecycle-audit.md`
35. `0040-reconnect-resume-design-mvp.md`
36. `0041-protocol-security-hardening-pass.md`
37. `0042-coverage-baseline-and-test-policy-audit.md`
38. `0043-map-contest-pressure-and-neutral-hazards.md`
39. `0044-neutral-crow-hazard-prototype.md`
40. `0045-visual-combat-and-objective-readability-polish.md`
41. `0046-demo-and-portfolio-polish.md`
42. `0047-repo-hygiene-and-ci-fixes.md`
43. `0048-scenario-config-single-source.md`
44. `0049-desktop-movement-reliability.md`
45. `0050-paired-config-driven-gameplay-scenarios.md`
46. `0051-config-driven-map-redesign.md`
47. `0052-config-driven-hazards-and-visualization.md`
48. `0053-objective-event-visibility.md`
49. `0054-desktop-and-mobile-visual-polish.md`
50. `0055-docs-for-config-driven-playable-milestone.md`
51. `0056-final-validation-gate.md`
52. `0057-pre-push-baseline-inspection.md`
53. `0058-desktop-mobile-scenario-consistency.md`
54. `0059-rematch-from-current-screen.md`
55. `0060-score-capture-winner-clarity.md`
56. `0061-objective-reset-staging.md`
57. `0062-config-driven-arena-route-redesign.md`
58. `0063-object-taxonomy-and-danger-semantics.md`
59. `0064-svg-hazard-visual-refresh.md`

Start multi-task Codex orchestration with `docs/agent-seeds/CODEX_ORCHESTRATOR_PROMPT.md`.
