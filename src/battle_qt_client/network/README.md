# Qt network

Network code stays separated from widgets.

Current implementation:

- `NetworkClient` is a `QObject` wrapper around asynchronous `QTcpSocket`.
- It owns raw TCP length-prefix framing and rejects zero, oversized, malformed, or client-originated server messages.
- It serializes outbound protocol envelopes through `battle_protocol::serializeEnvelope` and validates them with the current session phase before writing.
- It parses and validates server envelopes before emitting typed UI signals for auth, match join, snapshots, input acks, events, errors, and latency.
- It sends only player intentions: move, aim, attack, dash, interact, and stop.
- It does not own authoritative match state and does not link to `battle_core`.

No blocking calls are allowed on the UI thread.
