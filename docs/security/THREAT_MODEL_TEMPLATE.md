# Threat Model

## Scope

System under review:

- C++20 TCP battle server.
- Qt desktop client.
- Custom protocol over TCP.
- Optional public deployment on a cloud VM or TCP proxy platform.

Out of scope for MVP:

- Real account system.
- Payment data.
- Persistent personal profiles.
- Anti-cheat beyond server-authoritative command validation.

## Assets

- Server availability.
- Match state integrity.
- Player identity inside a session.
- Replay/log integrity.
- Developer credentials and deployment secrets.
- Repository quality and trustworthiness.

## Trust boundaries

```text
Untrusted Qt/CLI client
        |
        | TCP frames
        v
Protocol decoder and validator
        |
        v
Authoritative server session/match manager
        |
        v
Battle core
```

## Main threats

| Threat | Example | Required control |
|---|---|---|
| Malformed input crash | invalid length, invalid JSON | bounded framing, parse error handling, tests |
| Memory exhaustion | huge frame, slow reader | max frame, bounded queues, disconnect policy |
| Command spoofing | client sends another player id | session-bound identity, server-side ownership checks |
| State tampering | client sends hp/position | clients send intentions only, server owns state |
| Replay/log injection | terminal escape sequences | sanitize logs, avoid raw payload logging |
| Denial of service | command spam | rate limits, max clients, max matches |
| Race conditions | concurrent session/match updates | strand/event-loop model or explicit synchronization |
| Secret leakage | cloud keys in repo | `.gitignore`, no secrets in docs/tests/logs |
| Insecure deployment | open debug build publicly | minimal config, firewall docs, non-root container |

## Security assumptions

-

## Open security questions

-

## Security decisions

-
