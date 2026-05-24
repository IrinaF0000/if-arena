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

## Match loop model

`MatchLoop.hpp` adds the transport-neutral in-process match manager:

- `MatchId`, deterministic join codes, and session-owned player membership.
- `MatchManager::createMatch` and `joinMatch` start a two-player Objective Run match without sockets.
- `submitCommand` accepts intentions only, maps the backend-owned session player to `battle_core`, and rejects wrong sessions, claimed-player mismatches, duplicate or out-of-order sequence numbers, rate-limit excess, and bounded queue overflow.
- `tick` is the deterministic driver for tests and future workers: it drains a bounded command queue, applies commands to `BattleEngine`, queues readable event payloads for each matched session, and can defer snapshot fanout until the server `snapshotRate` is due.
- The default Objective Run match uses the canonical bottom-to-top arena, direct spawn-to-objective lanes, authoritative obstacle cells, configured mine/tower/crow hazards, automatic objective pickup/capture, a smooth per-tick player speed tuned for the live server tick rate, and an explicit 3600-tick match duration cap surfaced through `match_finished` events and `finished` snapshots.
- `MatchMetrics` tracks active/created matches, accepted/applied/rejected commands, snapshot/event broadcasts, disconnects, and queue overflows.

The current playable slice is intentionally two-player only. Slow sessions are handled by the existing bounded outbound queue: if snapshots/events accumulate past configured message or byte limits, the session is closed with `QueueOverflow`.
