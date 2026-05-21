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

## Session adapter

`WebSocketSession.hpp` provides the current local socket/session adapter:

- `WebSocketSessionAdapter` implements backend `IOutboundSession`.
- Inbound text messages are size-checked, then validated through `battle_protocol::parseEnvelope()`.
- Inbound messages are phase-validated with `validateClientEnvelope()` before backend code may handle them.
- Oversized, malformed, unknown, or out-of-order messages fail closed with a protocol disconnect reason.
- Handshake and idle timeout checks are explicit and testable.
- Outbound sends are bounded by message count and pending byte limits.
- The adapter stores sent payloads for deterministic tests; a concrete socket endpoint must flush those payloads to the WebSocket library/socket.
- `WebSocketListener` performs the local HTTP Upgrade handshake and returns `WebSocketConnection` objects.
- `WebSocketFrameDecoder` decodes masked client text frames and rejects oversized, unmasked, fragmented, or unsupported frames.
- `encodeWebSocketTextFrame()` emits unmasked server text frames.

Telegram identity is still backend-owned. The transport accepts raw auth envelopes and must pass Telegram `initData` to backend validation before marking a session authenticated.
