# battle_core Agent Rules

`battle_core` is the reusable deterministic game simulation library extracted from the old local simulation.

## Allowed responsibilities

- World state, entities, components, systems, actions, rules.
- Arena gameplay: movement, attacks, hazards, objectives, cooldowns.
- Deterministic tick/update logic.
- Snapshots and events as pure game data.
- Replay support that is independent of networking and UI.

## Forbidden dependencies

Do not add dependencies on:

- TCP, WebSocket, HTTP, TLS, or socket libraries.
- Qt, Telegram, browser APIs, JavaScript, TypeScript.
- Deployment, Docker, cloud providers, environment variables.
- Server session identity or transport-specific player IDs.

## Quality requirements

- Prefer value types, RAII, strong IDs, explicit ownership.
- No raw owning pointers.
- No hidden global mutable state.
- No nondeterminism unless injected through explicit seed/config.
- Tests must cover deterministic results for reference scenarios.
- Preserve behavior when extracting code from `external/battle_simulation_snapshot/`.

Read also: `docs/agent-rules/quality/CPP.md` and `docs/project/CORE_EXTRACTION_CHECKLIST.md`.
