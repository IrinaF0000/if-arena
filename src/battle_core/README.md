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
