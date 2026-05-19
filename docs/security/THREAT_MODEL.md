# Threat Model

## 1. System

IF Arena has:

- public raw TCP endpoint for Qt/CLI/load clients;
- public WebSocket endpoint for Telegram Mini App;
- Telegram Mini App frontend hosted over HTTPS;
- authoritative backend;
- extracted deterministic simulation core.

## 2. Assets

- backend availability;
- match integrity;
- player identity/session mapping;
- Telegram auth validity;
- server resources;
- logs without secrets;
- repository without committed secrets.

## 3. Attackers

- random internet client;
- modified Qt client;
- modified Mini App frontend;
- script sending malformed TCP frames;
- script sending malformed WebSocket messages;
- spam client;
- slow reader client;
- user replaying old Telegram init data;
- accidental buggy client.

## 4. Main risks and mitigations

### Malformed input crashes server

Mitigations:

- strict parser;
- boundary exception handling;
- negative tests;
- generic errors.

### Oversized messages exhaust memory

Mitigations:

- frame/message limits;
- reject before allocation;
- string limits.

### Slow client exhausts outgoing queues

Mitigations:

- bounded outgoing queues;
- drop obsolete snapshots;
- disconnect slow clients;
- metrics.

### Command spam overloads workers

Mitigations:

- per-session rate limits;
- pending command limits;
- aggregate metrics;
- disconnect repeated offenders.

### Client cheats by sending state

Mitigations:

- commands are intentions only;
- server validates ownership and rules;
- server owns all game state.

### Telegram identity spoofing

Mitigations:

- validate raw initData on backend;
- ignore initDataUnsafe for authority;
- check auth_date freshness;
- store no bot token in frontend.

### Secrets leaked in logs

Mitigations:

- log redaction;
- truncate raw payloads;
- no raw initData logging.

### Transport-specific bugs duplicate logic

Mitigations:

- transport abstraction;
- shared protocol parser;
- backend-only game authority;
- architecture review.

## 5. Open questions

- Exact library for WebSocket implementation.
- Exact Telegram initData validation implementation location.
- Public deployment platform and TLS termination path.
- Whether replay protection for Telegram auth is needed for MVP or only freshness check.

## 6. Review cadence

Update this file whenever:

- auth changes;
- a new transport is added;
- protocol message format changes;
- public deployment changes;
- logging behavior changes;
- resource limits change.
