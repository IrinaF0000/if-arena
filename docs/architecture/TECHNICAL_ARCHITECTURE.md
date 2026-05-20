# Technical Architecture

## Overview

IF Arena has one authoritative backend and two main UI clients:

- Qt desktop client over raw TCP;
- Telegram Mini App over WebSocket/WSS.

Both clients use the same gameplay model, protocol DTOs, and authoritative backend. The clients are presentation/input layers only.

```text
+-------------------+     raw TCP       +------------------------+
| Qt Desktop Client | -----------------> | battle_transport_tcp   |
+-------------------+                    +-----------+------------+
                                                |
+-------------------+     WebSocket      +--------v---------------+
| Telegram Mini App | -----------------> | battle_transport_ws    |
+-------------------+                    +-----------+------------+
                                                |
                                      +---------v----------+
                                      | battle_backend     |
                                      | sessions/matches   |
                                      | workers/metrics    |
                                      +---------+----------+
                                                |
                                      +---------v----------+
                                      | battle_core        |
                                      | static library     |
                                      +--------------------+
```

## Core constraints

- `battle_core` has no transport, Qt, Telegram, deployment, or server-process dependencies.
- `battle_protocol` is transport-independent.
- `battle_backend` owns authority, validation, sessions, match workers, metrics, rate limiting, and resource limits.
- `battle_transport_tcp` and `battle_transport_ws` are adapters.
- Qt and Telegram clients never own authoritative game rules.

Initial `battle_core` facade:

- `BattleEngine` owns in-process match state.
- `MatchConfig` creates authoritative match/player setup.
- `PlayerCommand` carries player intentions, not authoritative positions, HP, cooldowns, scores, or results.
- `CommandResult` reports accepted/rejected commands.
- `BattleSnapshot` and `BattleEvent` expose pure gameplay data for backend publication.

## Game authority

The server validates:

- player identity/session;
- match membership;
- movement limits;
- attack cooldown and hit result;
- dash cooldown;
- objective pickup/drop/capture;
- carrier slowdown;
- pickup lock;
- hazard effects;
- score and match end.

Clients send only intentions:

```text
move direction
aim direction
attack
dash
interact
stop
```

Clients never send authoritative position, HP, cooldown, score, objective owner, or match result.

## Coordinate systems

The backend uses canonical world coordinates.

Recommended MVP canonical map:

```text
width  = 21
height = 13
center = (10, 6)
red base/spawn near top
blue base/spawn near bottom
```

Clients may render player-oriented views:

- local player's base appears at the bottom;
- enemy base appears at the top;
- local input is transformed into canonical world commands;
- snapshots are transformed from canonical world positions into screen positions.

Replay, spectator, and debug tools use canonical orientation.

## Map model

The arena is authored on a logical grid, but movement is smooth.

Grid-authored data:

- walls/obstacles;
- base zones;
- spawn points;
- objective spawn;
- mines;
- towers;
- drone/crow path points.

World-space simulation:

- hero positions;
- movement vectors;
- dash movement;
- attack/hit checks;
- pickup radius;
- base capture checks.

## Match worker model

Each match is owned by exactly one match worker. This keeps mutable match state isolated and avoids shared mutable game state across worker threads.

```text
network sessions -> command queue -> match worker -> battle_core tick -> snapshots/events
```

The match worker applies accepted commands on tick boundaries or deterministic processing steps.

The in-process playable slice exposes this as `battle_backend::MatchManager`. It creates a two-player Objective Run match through a deterministic join code, owns the session-to-player mapping, rejects any client-claimed player mismatch, and publishes event/snapshot payloads through bounded outbound session queues. The current test driver calls `tick` directly; later transports should feed the same manager rather than owning match state.

## Tick loop

Initial settings:

```text
tick rate: 20 Hz
snapshot rate: 10-20 Hz
metrics interval: 1 second
```

The tick loop:

1. drains bounded command queues;
2. validates commands;
3. applies movement and actions;
4. updates hazards;
5. resolves objective state;
6. emits events;
7. publishes snapshots.

## Backend resource limits

The backend must enforce:

- max clients;
- max matches;
- max frame/message size;
- max pending write bytes per session;
- max pending commands per session;
- input rate limit per session;
- handshake timeout;
- idle timeout.

Slow clients must not cause unbounded memory growth. Snapshots may be coalesced.

## Transports

### Raw TCP

Used by:

- Qt desktop client;
- CLI debug client;
- load client.

Framing:

```text
uint32_be payload_size
payload_size bytes JSON
```

Transport must handle partial frames, combined frames, oversized frames, invalid JSON, disconnects, and slow readers.

### WebSocket

Used by Telegram Mini App.

Requirements:

- public deployment uses WSS;
- message size limits;
- backend-side Telegram `initData` validation;
- same protocol DTOs as TCP;
- no frontend secrets;
- no client-side authority.

## Qt client

Qt Widgets desktop client.

Required structure:

```text
MainWindow
BattleViewWidget / BattleScene
NetworkClient
ProtocolClient
InputMapper
CoordinateTransform
```

Rules:

- no blocking socket calls in UI thread;
- UI sends intentions, not state;
- NetworkClient is separate from widgets;
- rendering uses server snapshots;
- local orientation transform is client-side only.

## Telegram Mini App

TypeScript frontend.

Required structure:

```text
TelegramBridge
WebSocketClient
ProtocolClient
ArenaCanvas
TouchControls
InputMapper
CoordinateTransform
HudView
```

Rules:

- strict TypeScript;
- no backend secrets;
- no authoritative game rules;
- WebSocket only;
- backend validates Telegram auth;
- runtime validation for inbound messages.

## Observability

Backend must expose or log structured metrics:

- active TCP connections;
- active WebSocket connections;
- active matches;
- commands/s;
- snapshots/s;
- frame/message reject count;
- invalid command count;
- queue sizes;
- tick duration;
- p50/p95/p99 latency;
- disconnect reasons.

Logs must be safe and redact secrets/tokens/auth payloads.

## Security boundaries

The most important trust boundary is between client transports and backend session handling.

Every inbound payload is untrusted until:

1. framing/message size is validated;
2. JSON/schema is validated;
3. session/auth state is checked;
4. command authority is checked;
5. gameplay validation passes.

Telegram `initDataUnsafe` must not be trusted. Raw Telegram `initData` is sent to the backend and validated there.
