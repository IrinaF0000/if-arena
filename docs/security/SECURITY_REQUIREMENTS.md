# Security Requirements

## 1. Security goals

- Keep the backend stable under malformed and hostile network input.
- Keep gameplay server-authoritative.
- Prevent clients from mutating state directly.
- Prevent unbounded memory growth from slow/spam clients.
- Protect Telegram Mini App identity validation.
- Avoid leaking secrets or sensitive auth data.
- Keep public demo deployment conservative.

## 2. Trust boundaries

```text
Raw TCP client -> untrusted
WebSocket client -> untrusted
Telegram Mini App frontend -> untrusted
Telegram initDataUnsafe -> untrusted
Telegram raw initData -> untrusted until backend validation succeeds
Server config -> trusted only if deployed securely
battle_core -> trusted internal library
```

## 3. Required controls

### 3.1 Protocol parsing

- Limit message/frame size.
- Limit strings.
- Validate required fields.
- Validate enum values.
- Validate numeric ranges.
- Reject unknown message types.
- Reject invalid message order.
- Catch parse errors at boundary.
- Do not crash on invalid input.

### 3.2 TCP transport

- Handle partial frames.
- Handle combined frames.
- Reject oversized frame length before allocation.
- Apply handshake timeout.
- Apply idle timeout.
- Bound outgoing queue.
- Disconnect or degrade slow clients.

### 3.3 WebSocket transport

- Limit message size.
- Apply handshake timeout.
- Apply idle timeout.
- Use WSS in public deployment.
- Bound outgoing queue.
- Validate every message through protocol parser.
- Disconnect or degrade slow clients.

### 3.4 Telegram auth

- Validate raw `initData` on backend.
- Check signature/hash.
- Check freshness of `auth_date`.
- Do not trust frontend `initDataUnsafe`.
- Do not put bot token in frontend.
- Redact raw init data from logs.
- Fail closed on validation failure.

Implementation note:

- `TelegramAuthValidator` currently performs backend-side size, HMAC-SHA256 hash, and freshness checks using a bot token supplied by secure runtime configuration.
- Replay protection and session-token issuance are still required follow-up work.

### 3.5 Session and authorization

- Backend generates session ids.
- Backend maps authenticated identity to internal player id.
- Clients cannot select arbitrary player id.
- Commands must be checked against session and match membership.
- Commands must be checked against unit ownership/team.
- Commands before auth or match join must be rejected.

### 3.6 Gameplay authority

Clients may send:

```text
move direction
aim direction/target
attack request
dash request
interact request
stop request
```

Clients must not send authoritative:

```text
position
velocity except as input direction
hp
damage
cooldown
score
team
match result
server tick
object spawn/despawn
```

### 3.7 Resource exhaustion

Must bound:

- connections;
- unauthenticated sessions;
- authenticated sessions;
- matches;
- commands per second;
- pending commands per session;
- outgoing bytes per session;
- frame/message size;
- display name length;
- Telegram init data length;
- reconnect attempts.

### 3.8 Logging

- No raw auth data.
- No tokens.
- No bot token.
- No full hostile payloads by default.
- Truncate long values.
- Public errors must be generic.

## 4. Required tests

Security-sensitive tasks must add relevant tests:

- malformed JSON;
- oversized TCP frame;
- oversized WebSocket message;
- unknown message type;
- command before auth;
- command before join;
- command for another player's unit;
- rate limit exceeded;
- slow reader;
- invalid Telegram hash;
- stale Telegram auth date;
- oversized Telegram init data;
- replayed Telegram auth if replay protection exists.

## 5. Deployment requirements

Public demo:

- Use conservative config.
- Use firewall to expose only required ports.
- Use WSS/HTTPS for Telegram Mini App.
- Keep raw TCP optional and documented.
- Do not commit secrets.
- Use environment variables or secret manager for bot token.
- Disable development demo auth unless explicitly intended.

## 6. References

- Telegram Mini Apps validation docs: https://core.telegram.org/bots/webapps
- WebSocket API notes: https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API
- NIST Secure Software Development Framework: https://csrc.nist.gov/pubs/sp/800/218/final
- OWASP ASVS: https://owasp.org/www-project-application-security-verification-standard/
- SEI CERT C++ Coding Standard: https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=88046682
