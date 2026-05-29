# Protocol

## 1. Purpose

The protocol defines transport-independent game messages shared by:

- raw TCP clients;
- WebSocket clients;
- backend sessions;
- load testing tools.

TCP and WebSocket use different framing, but payload semantics should stay aligned.

## 2. Transports

### 2.1 Raw TCP

Used by:

- Qt client;
- CLI client;
- load client.

Framing:

```text
uint32_be payload_size
payload_size bytes of UTF-8 JSON
```

The standalone C++ codec is `src/battle_transport_tcp/TcpFrameCodec.hpp`.
`encodeFrame()` writes the big-endian length prefix. `TcpFrameDecoder::feed()` accepts arbitrary byte chunks and may emit zero, one, or many complete payloads.
Invalid zero-length frames and oversized length prefixes put the decoder into an explicit closed/error state.

TCP-specific requirements:

- never assume one read equals one message;
- handle partial frames;
- handle multiple frames in one read;
- reject oversized frame before allocating payload buffer;
- close session on framing violation.

The current local TCP server sends and receives these same JSON envelopes over the length-prefixed frame codec. A malformed frame or invalid envelope receives a structured `error` when possible, then the connection is closed.

### 2.2 WebSocket

Used by Telegram Mini App.

Framing:

```text
one WebSocket text or binary message = one protocol payload
```

WebSocket-specific requirements:

- enforce max message size;
- validate every payload through the same protocol parser;
- use WSS in public deployment;
- apply per-session rate limits and bounded outgoing queues;
- handle browser clients that may buffer messages.

## 3. Message envelope

All payloads use this envelope:

```json
{
  "version": 1,
  "type": "input_command",
  "requestId": "client-generated-id",
  "sessionSeq": 42,
  "payload": {}
}
```

Fields:

- `version`: protocol version.
- `type`: message type string.
- `requestId`: optional but recommended for request/ack correlation.
- `sessionSeq`: monotonic per-session sequence number for client-originated gameplay messages.
- `payload`: type-specific object.

The C++ helper API in `src/battle_protocol/Protocol.hpp` exposes this as `Envelope`.
`parseEnvelope()` validates the top-level object and returns a `ParseResult`; parser failures are structured `ProtocolError` values and must not escape transport code as uncaught exceptions.
`validateClientEnvelope()` validates client-originated payloads against the current session phase before backend code handles them.
`serializeEnvelope()` emits the same transport-neutral JSON envelope.

Recognized wire `type` values:

```text
auth_request
auth_result
create_match
join_match
start_next_match
match_joined
input_command
input_ack
snapshot
event_batch
handshake
ping
pong
error
```

Unknown message types are rejected before payload-specific handling.

## 4. Size limits

Initial recommended defaults:

```text
MAX_TCP_FRAME_BYTES = 64 KiB
MAX_WS_MESSAGE_BYTES = 64 KiB
MAX_DISPLAY_NAME_BYTES = 64
MAX_MATCH_CODE_BYTES = 32
MAX_TELEGRAM_INIT_DATA_BYTES = 4096
MAX_PENDING_WRITE_BYTES_PER_SESSION = 1 MiB
MAX_PENDING_COMMANDS_PER_SESSION = 64
MAX_INPUT_COMMANDS_PER_SECOND = 30
```

All limits must be configurable and have conservative public defaults.

Initial C++ parser defaults are:

```text
maxMessageBytes = 64 KiB
maxPayloadBytes = 60 KiB
maxStringBytes = 4096
maxTypeBytes = 64
maxRequestIdBytes = 128
```

Transport layers must still enforce their own frame/message limits before storing or passing payload bytes to the parser.

## 5. Authentication messages

### 5.1 Local/demo auth

For CLI/Qt local demo:

```json
{
  "version": 1,
  "type": "auth_request",
  "requestId": "r1",
  "payload": {
    "mode": "demo",
    "displayName": "Anna"
  }
}
```

### 5.2 Telegram auth

Telegram Mini App sends raw `initData` to backend:

```json
{
  "version": 1,
  "type": "auth_request",
  "requestId": "r1",
  "payload": {
    "mode": "telegram",
    "initData": "query_id=...&user=...&auth_date=...&hash=..."
  }
}
```

Backend must validate `initData` before trusting Telegram identity.

### 5.3 Auth result

```json
{
  "version": 1,
  "type": "auth_result",
  "requestId": "r1",
  "payload": {
    "ok": true,
    "playerId": "p_123",
    "displayName": "Anna"
  }
}
```

## 6. Match messages

### 6.1 Create match

```json
{
  "version": 1,
  "type": "create_match",
  "requestId": "r2",
  "payload": {
    "mode": "objective_run",
    "scenario": "arena_small_objective_run"
  }
}
```

### 6.2 Join match

```json
{
  "version": 1,
  "type": "join_match",
  "requestId": "r3",
  "payload": {
    "matchCode": "ABCD"
  }
}
```

### 6.3 Match joined

```json
{
  "version": 1,
  "type": "match_joined",
  "requestId": "r3",
  "payload": {
    "matchId": "m_1",
    "matchCode": "ABCD",
    "scenario": {
      "id": "arena_small_objective_run",
      "mode": "objective_run",
      "version": 1,
      "source": "server_config"
    },
    "team": "blue",
    "localView": {
      "ownBaseAtBottom": true,
      "inputOrientation": "team_local"
    }
  }
}
```

### 6.4 Start next match

After an authoritative snapshot reports `finished: true`, either participant may request a fresh match with the same server-owned scenario and participants:

```json
{
  "version": 1,
  "type": "start_next_match",
  "requestId": "r4",
  "payload": {
    "matchId": "m_1"
  }
}
```

The server rejects requests before match completion, from non-participants, or with client-owned authority fields. A successful request emits a new `match_joined` payload and fresh snapshots; clients must treat the returned `matchId` as a new match and reset gameplay command sequencing.

## 7. Input commands

Client sends intentions only.

```json
{
  "version": 1,
  "type": "input_command",
  "requestId": "r4",
  "sessionSeq": 10,
  "payload": {
    "matchId": "m_1",
    "command": {
      "kind": "move",
      "direction": { "x": 1, "y": 0 }
    }
  }
}
```

Supported MVP commands:

```text
move
aim
attack
dash
interact
stop
```

Movement and aim directions are sent in canonical world coordinates. A player-oriented client must transform local input into canonical direction before sending the command.
Direction coordinates are integer unit components in `[-1, 0, 1]`; zero vectors and oversized values are rejected.
`attack` and `dash` carry intent only. The server decides hits, damage, cooldowns, dash distance, objective drops, and match results.

Forbidden client claims:

```text
position
hp
cooldown
damage
score
team
match result
server time
```

### 7.1 Input ack

```json
{
  "version": 1,
  "type": "input_ack",
  "requestId": "r4",
  "payload": {
    "accepted": true,
    "serverTick": 1004
  }
}
```

Rejected command:

```json
{
  "version": 1,
  "type": "input_ack",
  "requestId": "r4",
  "payload": {
    "accepted": false,
    "reason": "rate_limited",
    "serverTick": 1005
  }
}
```

## 8. Snapshots

Snapshots use canonical world coordinates. Player clients transform these coordinates for local player-oriented rendering.

```json
{
  "version": 1,
  "type": "snapshot",
  "payload": {
    "matchId": "m_1",
    "serverTick": 1200,
    "serverTimeMs": 60000,
    "scenario": {
      "id": "arena_small_objective_run",
      "mode": "objective_run",
      "version": 1,
      "source": "server_config"
    },
    "map": {
      "width": 21,
      "height": 13,
      "center": { "x": 10, "y": 6 },
      "symmetry": "rotational_180"
    },
    "obstacles": [
      {
        "id": "obstacle_7_5",
        "kind": "blocking_obstacle",
        "visualId": "obstacle_block",
        "x": 7,
        "y": 5,
        "blocksMovement": true,
        "damage": 0,
        "causesDrop": false,
        "rangeRadius": 0,
        "cooldownTicks": 0,
        "cooldown": 0,
        "team": "neutral"
      }
    ],
    "players": [
      {
        "playerId": "p_1",
        "team": "blue",
        "displayName": "Anna",
        "score": 1
      }
    ],
    "objective": {
      "id": "objective_center",
      "kind": "carry_objective",
      "state": "carried",
      "x": 10.5,
      "y": 6.0,
      "carrierUnitId": "u_1",
      "pickupLockedUntilTick": null
    },
    "objects": [
      {
        "id": "u_1",
        "kind": "hero",
        "team": "blue",
        "x": 10.5,
        "y": 8.0,
        "hp": 80,
        "maxHp": 100,
        "state": "alive",
        "isCarrier": true,
        "speedMultiplier": 0.8
      },
      {
        "id": "h_1",
        "kind": "mine",
        "visualId": "hazard_mine",
        "team": "neutral",
        "x": 7.0,
        "y": 4.0,
        "radius": 0.7,
        "range": 1.0,
        "damage": 12,
        "effect": "damage_drop_objective",
        "trigger": "proximity",
        "icon": "hazard_mine",
        "blocksMovement": false,
        "causesDrop": true,
        "rangeRadius": 0.7,
        "cooldownTicks": 30,
        "cooldown": 0,
        "triggered": false,
        "state": "armed"
      }
    ]
  }
}
```

The playable snapshot also exposes authoritative object semantics in `payload.obstacles` and `payload.hazards`.
This is currently a snapshot-only model: static fields (`id`, `kind`, `visualId`, movement blocking, damage, drop behavior, range/radius, ownership, effect/trigger, and configured cooldown) are sent beside dynamic fields (`x`, `y`, `cooldown`, `triggered`) until a dedicated static arena metadata message is introduced.

Object taxonomy used by current snapshots:

| Kind | Sent As | Blocks Movement | Damage | Causes Objective Drop | Range/Radius | Cooldown | Movement | Ownership |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| blocking obstacle | `payload.obstacles[]`, `kind: "blocking_obstacle"` | yes | 0 | no | 0 | 0 | static | neutral |
| damage hazard | `payload.hazards[]`, `effect: "damage"` | no | `damage` | no | `rangeRadius` | `cooldownTicks`/`cooldown` | static | neutral |
| damage + drop hazard | `payload.hazards[]`, `effect: "damage_drop_objective"` | no | `damage` | yes | `rangeRadius` | `cooldownTicks`/`cooldown` | static | neutral |
| tower bot | `payload.hazards[]`, `kind: "tower"` | no | `damage` | from `causesDrop` | `rangeRadius` | `cooldownTicks`/`cooldown` | static ranged trigger | neutral |
| mine bot | `payload.hazards[]`, `kind: "mine"` | no | `damage` | from `causesDrop` | `rangeRadius` | `cooldownTicks`/`cooldown` | static proximity trigger | neutral |
| crow bot | `payload.hazards[]`, `kind: "crow"` | no | `damage` | from `causesDrop` | `rangeRadius` | `cooldownTicks`/`cooldown` | static proximity/range trigger | neutral |
| base | map/scenario owned capture zone rendered by clients | no | 0 | no | scenario capture zone | none | static | blue/red |
| objective | `payload.objective` | no | 0 | n/a | pickup/carry state | pickup/respawn ticks | static/carried/respawning | neutral |
| player | `payload.players[]` | actor collision is server-owned | combat rules | can carry/drop objective by server events | attack/dash cooldowns | player cooldown fields | player input intentions only | blue/red |

Clients must render from this metadata and may show compact labels, range circles, and legends. Clients must not derive authoritative damage, drop, blocking, cooldown, or ownership rules from visual colors or local hardcoded hazard kind behavior.

## 9. Event batches

```json
{
  "version": 1,
  "type": "event_batch",
  "payload": {
    "matchId": "m_1",
    "serverTick": 1201,
    "events": [
      {
        "kind": "damage",
        "targetId": "u_2",
        "amount": 10
      },
      {
        "kind": "objective_dropped",
        "objectiveId": "objective_center",
        "previousCarrierId": "u_1",
        "reason": "carrier_hit"
      },
      {
        "kind": "objective_captured",
        "playerId": "p_1",
        "score": 2
      }
    ]
  }
}
```

## 10. Errors

```json
{
  "version": 1,
  "type": "error",
  "requestId": "r9",
  "payload": {
    "code": "invalid_message",
    "message": "Invalid message"
  }
}
```

Public error messages must not leak stack traces, internal paths, secret values, or parser internals.

## 11. Required negative tests

Protocol changes must include tests for:

- malformed JSON;
- unknown message type;
- missing fields;
- invalid field types;
- oversized message;
- oversized string;
- invalid enum;
- invalid sequence;
- command before auth;
- command before joining match;
- command for another player;
- command rate exceeded;
- replayed Telegram auth where applicable.
