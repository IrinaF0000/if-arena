# battle_protocol

Transport-independent protocol DTOs, serialization, parsing, validation, limits, and error codes.

Allowed:
- message envelope;
- protocol versions;
- message types;
- validation helpers;
- schema-like checks.

Forbidden:
- socket code;
- WebSocket code;
- gameplay rule execution;
- Qt widgets;
- Telegram frontend code.

## Public helpers

`Protocol.hpp` currently provides:

- `ProtocolLimits` for bounded payload/string/message sizes;
- `MessageType` and stable wire type names;
- `Envelope` for the transport-neutral JSON envelope;
- `parseEnvelope()` returning `ParseResult` with structured `ProtocolError`;
- `serializeEnvelope()` returning `SerializeResult`.

Parser errors are data, not uncaught exceptions. Callers should reject or close abusive transport sessions without logging raw payloads.
