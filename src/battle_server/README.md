# battle_server

Authoritative TCP backend.

The server links with `battle_core` and `battle_protocol`. It does not use the old project executable and does not depend on `external/battle_simulation_snapshot/`.

Key responsibilities:

- async TCP sessions;
- length-prefixed protocol handling;
- command validation at network/session level;
- match workers and tick loop;
- bounded queues and backpressure;
- metrics and graceful shutdown.
