# Transport Abstraction

## Goal

Support multiple client transports without duplicating backend game logic.

MVP transports:

- raw TCP for Qt/CLI/load clients;
- WebSocket for Telegram Mini App.

## Design

Transport implementations adapt network connections into a common backend session interface.

```cpp
class BackendSessionSink {
public:
    virtual void onMessage(SessionId session, ProtocolMessage message) = 0;
    virtual void onDisconnected(SessionId session, DisconnectReason reason) = 0;
};

class TransportSession {
public:
    virtual void send(ProtocolMessage message) = 0;
    virtual void close(DisconnectReason reason) = 0;
};
```

Exact names may differ, but the dependency direction must stay:

```text
transport -> backend interface
backend -> abstract send/close callback
backend must not know socket details
```

Current C++ backend names:

- `IOutboundSession`: abstract send/close callback implemented by transport adapters or fake tests.
- `BackendSession`: transport-neutral per-connection state, auth state, backend-owned player identity, and bounded outbound queue.
- `SessionRegistry`: creates, finds, closes, and counts sessions.
- `BackendLimits`: per-session and registry bounds for active sessions and pending outbound messages/bytes.

Transport code must not set authoritative gameplay state. It forwards validated protocol messages into backend code; backend authentication assigns `PlayerId`.

## Shared responsibilities

Both transports must enforce:

- max message size;
- handshake timeout;
- idle timeout;
- rate limit hooks;
- bounded outgoing queue;
- safe disconnect;
- metrics labeling by transport.

The initial backend queue policy is fail-closed: if a session exceeds pending message or byte limits, backend closes the abstract outbound session with `QueueOverflow`.

## Current raw TCP vertical slice

Task 0025 wires the local raw TCP path into backend sessions:

- `battle_server_app` starts `TcpListener`, accepts local clients, and creates one backend `SessionRegistry` session per connection.
- Client messages are parsed with `battle_protocol::parseEnvelope()` and phase-validated with `validateClientEnvelope()` before backend calls.
- `auth_request`, `create_match`, `join_match`, `input_command`, `ping`, and `pong` are handled in the TCP composition loop.
- Authoritative snapshots/events are produced by `battle_backend` and sent back through the backend outbound queue, not by client code.
- Malformed JSON, unknown message types, oversized frames, handshake timeout, idle timeout, and bounded outbound queues fail closed.

This slice is local/demo infrastructure. Public hardening, async I/O scaling, richer metrics, and production deployment remain later tasks.

## Transport-specific responsibilities

TCP:

- byte stream framing;
- partial frame accumulation;
- length prefix validation.

WebSocket:

- WebSocket frame receive/send;
- WSS deployment;
- Telegram auth flow.

Current local WebSocket slice:

- `WebSocketSessionAdapter` implements backend `IOutboundSession`.
- Each received WebSocket text message is treated as one protocol payload.
- Message size is checked before protocol parsing.
- Payload validation uses `battle_protocol::parseEnvelope()`.
- The transport does not validate Telegram identity; it forwards auth payloads toward backend-owned validation.
- `battle_server_app` includes a local HTTP Upgrade listener for Telegram Mini App/browser development when TCP is disabled and WebSocket is enabled in config.

## Forbidden duplication

Do not duplicate:

- match rules;
- command validation;
- player ownership checks;
- scoring;
- hazard logic;
- rate limiter implementation unless transport-specific adapter is required;
- metrics naming unless labels differ.

## Testing

Every backend flow should be testable with fake sessions without opening real sockets.

Every transport should have integration tests for network-specific edge cases.
