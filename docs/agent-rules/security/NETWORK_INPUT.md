# Network Input Security

All network input is untrusted.

## Required

- Enforce max frame/message size before allocation.
- Enforce handshake timeout and idle timeout.
- Enforce per-session command rate limits.
- Reject unknown message kinds.
- Validate field types and ranges.
- Use structured errors without leaking internals.
- Disconnect abusive clients according to policy.
- Do not parse unbounded JSON.
- Do not log raw payloads by default.

## Tests required for protocol/transport tasks

- partial frame;
- combined frames;
- oversized frame;
- malformed JSON;
- unknown message type;
- invalid field types;
- command for another player's unit;
- spam client;
- slow reader.
