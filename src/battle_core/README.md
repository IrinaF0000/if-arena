# battle_core

Static library extracted from the original local battle simulation.

Allowed:
- deterministic game state;
- gameplay systems;
- actions/rules;
- hazards;
- objectives;
- replay/events;
- snapshots.

Forbidden:
- TCP;
- WebSocket;
- Qt;
- Telegram;
- process-level server code;
- deployment;
- network sessions.

This module must remain reusable and testable without sockets.

## Boundary layers

`battle_core` is split conceptually into:

- minimal foundation primitives such as ids, geometry, ticks, and small value/result types;
- gameplay features such as movement, objective rules, combat, dash, and hazards;
- scenario/map config such as the current 21x13 Objective Run defaults;
- pure view/input transforms that convert player-oriented directions without deciding gameplay outcomes.

IO, JSON/file parsing, environment variables, sockets, Qt, Telegram, backend sessions, and deployment wiring belong outside this module.

Temporary MVP exceptions are documented in `docs/architecture/BATTLE_CORE_BOUNDARIES.md`: `ArenaConfig.hpp` may currently contain default Objective Run helpers, and player-view transforms may live in core until a dedicated refactor extracts them. Do not grow those exceptions.

## Public facade

`BattleEngine.hpp` exposes the first server-facing core facade:

- `MatchConfig` and `PlayerConfig` define authoritative match setup.
- `PlayerCommand` accepts player intentions only: move, stop, attack, dash, and optional interact compatibility.
- `CommandResult` reports command acceptance or rejection.
- `BattleSnapshot` is immutable snapshot data for backend fanout.
- `BattleEvent` reports deterministic state changes from `tick()`.

Clients and transports must not construct authoritative state directly; they send intentions to backend code that later calls this facade.

Movement is fixed-step and server-authoritative:

- players have canonical world positions, desired movement, team, spawn, and own-base state;
- move commands set desired movement and stop commands clear it;
- ticks advance smooth world positions deterministically;
- arena bounds and obstacle cells block movement in core before snapshots are emitted.

Objective Run rules are also server-authoritative:

- optional match objective config defines spawn, pickup radius, carrier slowdown, pickup lock, respawn delay, and score limit;
- pickup is automatic on contact when objective state, distance, and pickup lock allow it;
- `interact` commands remain accepted only as compatibility intentions and do not let clients claim objective ownership;
- server-side systems can drop the current carrier through the core facade for later combat/hazard tasks;
- captures are validated against the carrier's own base, update score, respawn or end the match, and emit deterministic events.

Combat, dash, and MVP hazards are core-owned:

- attack commands carry an aim direction only; hit detection, damage, cooldowns, and objective drops are resolved by `BattleEngine`;
- dash commands carry a direction only; dash distance, cooldowns, arena bounds, and obstacle-safe movement are resolved by core;
- mine and tower hazards are deterministic match config values and emit telegraph/hit events before applying server-owned damage.
- obstacle cells are included in snapshots so clients can render only server-authoritative blockers.

`ArenaConfig.hpp` defines the Objective Run arena model for the playable slice:

- canonical 21x13 arena dimensions;
- base zones, player spawn cells, obstacle cells, hazard configs, and objective spawn cell;
- 180-degree rotational symmetry helpers;
- player-view coordinate/input transform helpers that keep server coordinates canonical;
- validation for bounds, required objects, duplicate/blocked cells, hazards, and symmetric layout.

The arena config is pure core data. It does not parse files, read environment variables, or depend on transport/client code.
