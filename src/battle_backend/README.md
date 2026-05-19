# battle_backend

Authoritative backend application layer.

Owns:
- sessions;
- authenticated player identity;
- match manager;
- match workers;
- command queues;
- rate limiting;
- backpressure;
- metrics;
- routing between transports and battle_core.

Does not own raw socket/WebSocket details. Does not render UI.
