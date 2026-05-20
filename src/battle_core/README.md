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
