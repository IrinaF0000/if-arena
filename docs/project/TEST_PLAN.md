# Test Plan

## 1. Test strategy

The project must test:

- extracted core determinism;
- protocol validation;
- TCP framing;
- WebSocket message handling;
- backend authority;
- security-sensitive invalid input;
- Qt manual flows;
- Telegram Mini App frontend checks;
- load behavior.

## 2. Unit tests

### battle_core

- create match;
- apply valid command;
- reject invalid command;
- deterministic tick sequence;
- hazard behavior;
- objective pickup/drop/capture/score;
- carrier slowdown;
- pickup lock;
- player-oriented coordinate transforms;
- map rotational symmetry validation.

### battle_protocol

- valid envelope parse;
- malformed JSON rejected;
- unknown type rejected;
- missing fields rejected;
- invalid field type rejected;
- oversized string rejected;
- invalid enum rejected.

### battle_backend

- session created/closed;
- command before auth rejected;
- command before match join rejected;
- command for another player rejected;
- rate limiting;
- bounded outgoing queue;
- match worker owns state.

### battle_transport_tcp

- partial frame;
- combined frames;
- oversized frame length;
- invalid payload;
- disconnect on violation.

### battle_transport_ws

- valid message path;
- oversized message;
- malformed message;
- command before auth;
- slow client behavior where testable.

## 3. Security tests

Required cases:

- invalid Telegram hash;
- stale Telegram auth date;
- oversized Telegram init data;
- missing Telegram user;
- replayed auth data if replay protection exists;
- spam commands;
- slow clients;
- malformed payloads;
- client claims position/HP/score/team/result;
- client claims objective carrier/drop/capture state.

## 4. Integration tests

- CLI client connects to server app locally.
- Two fake clients create/join match.
- Two clients send movement/action commands.
- Server broadcasts snapshots.
- Carrier hit drops objective.
- Pickup lock prevents immediate re-pickup.
- Capture updates score and resets objective.
- Invalid client does not crash server.
- TCP and WebSocket clients can connect to same backend in the same process.

## 5. Qt manual tests

Checklist under `tests/manual/`:

- connect/disconnect;
- create/join match;
- movement keys;
- mouse aim;
- attack/dash/interact;
- connection loss;
- invalid server address;
- visual readability;
- own base at bottom local view for both teams;
- replay/debug canonical orientation.

## 6. Telegram Mini App tests

Automated:

- `npm run typecheck`;
- `npm run lint`;
- type guards for inbound messages;
- WebSocket client state machine tests if test framework is added.

Manual:

- open Mini App locally;
- connect to local WebSocket;
- send auth request;
- join match;
- mobile layout;
- desktop layout;
- disconnect/reconnect state;
- no secrets in bundle.

## 7. Load tests

Scenarios:

- normal clients;
- command spam;
- slow readers;
- malformed clients;
- mixed TCP/WebSocket clients as stretch.

Reports saved under:

```text
reports/load/
```

## 8. Acceptance

No MVP feature is complete until:

- unit/integration tests exist where applicable;
- security-negative tests exist for network/auth changes;
- manual UI checklist is updated for UI changes;
- load/performance impact is assessed for queues/workers/transports.
