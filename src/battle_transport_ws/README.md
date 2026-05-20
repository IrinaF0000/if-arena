# battle_transport_ws

WebSocket transport adapter for Telegram Mini App.

Owns:
- WebSocket connection lifecycle;
- message size limits;
- keepalive;
- WebSocket-to-backend session adapter.

Must use shared protocol validation.
Must not trust Telegram identity by itself.
Must not implement gameplay rules.

## Session skeleton

`WebSocketSession.hpp` provides the current no-socket skeleton:

- `WebSocketSessionAdapter` implements backend `IOutboundSession`.
- Inbound text messages are size-checked, then validated through `battle_protocol::parseEnvelope()`.
- Oversized inbound messages fail closed with a protocol disconnect reason.
- Malformed protocol messages return explicit errors and do not log raw payloads.
- Outbound sends are bounded by the same message size limit.

No WebSocket library is selected yet. A later integration packet should add the actual async WebSocket endpoint and keep this adapter boundary.
