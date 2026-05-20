# battle_transport_tcp

Raw TCP transport adapter.

Owns:
- async accept/read/write;
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

Socket integration must feed received bytes into this codec and pass complete payloads to `battle_protocol::parseEnvelope()`.
