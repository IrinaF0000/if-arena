# battle_transport_tcp

Raw TCP transport adapter.

Owns:
- accept/read/write for the local raw TCP slice;
- length-prefixed framing;
- partial/combined frame handling;
- TCP connection lifecycle;
- transport metrics labels.

Must forward parsed protocol messages to battle_backend.
Must not implement gameplay rules.

## Frame codec

`TcpFrameCodec.hpp` implements the standalone pre-socket codec:

- frame format: `uint32_be payload_size` followed by payload bytes;
- `encodeFrame()` rejects empty or oversized payloads;
- `TcpFrameDecoder::feed()` supports partial and combined frames;
- oversized or invalid length prefixes close the decoder error state before payload allocation;
- decoder buffering is bounded by `TcpFrameLimits`.

Socket integration feeds received bytes into this codec and passes complete payloads to `battle_protocol::parseEnvelope()`.

## Local TCP socket adapter

`TcpListener` and `TcpConnection` provide the current portable socket wrapper used by `battle_server_app`:

- `TcpListener::bindAndListen()` starts a bounded local listener.
- `TcpConnection::readFrame()` handles partial and combined stream reads through `TcpFrameDecoder`.
- `TcpConnection::sendFrame()` writes a length-prefixed JSON payload.
- receive/send timeouts are configured by the server app from local config.

Framing violations close the connection. Oversized length prefixes are rejected before payload allocation.
