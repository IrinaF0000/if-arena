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

## Transport-specific responsibilities

TCP:

- byte stream framing;
- partial frame accumulation;
- length prefix validation.

WebSocket:

- WebSocket frame receive/send;
- WSS deployment;
- Telegram auth flow.

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
