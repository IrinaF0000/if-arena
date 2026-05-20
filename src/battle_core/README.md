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

## Public facade

`BattleEngine.hpp` exposes the first server-facing core facade:

- `MatchConfig` and `PlayerConfig` define authoritative match setup.
- `PlayerCommand` accepts player intentions only, currently move/stop plus rejected placeholders for attack/interact.
- `CommandResult` reports command acceptance or rejection.
- `BattleSnapshot` is immutable snapshot data for backend fanout.
- `BattleEvent` reports deterministic state changes from `tick()`.

Clients and transports must not construct authoritative state directly; they send intentions to backend code that later calls this facade.

`ArenaConfig.hpp` defines the Objective Run arena model for the playable slice:

- canonical 21x13 arena dimensions;
- base zones, player spawn cells, obstacle cells, and objective spawn cell;
- 180-degree rotational symmetry helpers;
- validation for bounds, required objects, duplicate/blocked cells, and symmetric layout.

The arena config is pure core data. It does not parse files, read environment variables, or depend on transport/client code.
