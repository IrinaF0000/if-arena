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
