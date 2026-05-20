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

## Session model

`Session.hpp` provides the first transport-neutral backend session layer:

- `SessionId`, `ConnectionId`, and backend-owned `PlayerId` strong IDs.
- `IOutboundSession` as the only transport callback surface for send/close.
- `BackendSession` with connected/authenticated/closed state.
- Bounded outgoing queue limits through `BackendLimits`.
- `SessionRegistry` for creating, finding, closing, and counting sessions.

Client messages are rejected until backend authentication assigns a `PlayerId`.
Slow or overflowing outbound queues close the abstract outbound session with `QueueOverflow`.
