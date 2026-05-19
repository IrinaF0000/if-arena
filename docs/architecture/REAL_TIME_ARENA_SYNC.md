# Real-time Arena Synchronization

## Principle

The server is authoritative. Clients send input intentions. The server validates and applies them on ticks, then sends snapshots/events.

```text
client input -> transport command -> server validation -> tick update -> snapshot -> client render
```

## MVP sync approach

Use simple authoritative snapshots.

- Client sends movement direction or action command.
- Server applies command on the next tick or command processing step.
- Server sends regular snapshots.
- Client renders latest snapshot.
- Optional local prediction may be added later, but is not required for MVP.

## Player-oriented rendering

The server sends canonical world coordinates. Each client may render a local orientation where the local player's base is at the bottom.

Client responsibilities:

- transform canonical snapshot positions to local screen coordinates;
- transform local input directions to canonical world directions;
- keep replay/debug views canonical;
- never send local screen coordinates as authoritative state.

## Command sequence

Client commands should include a monotonically increasing client-local sequence number.

The server uses it for:

- acknowledgement;
- latency measurement;
- duplicate detection;
- debugging.

The server must not use client sequence number as authority over game state.

## Input commands

Supported MVP commands:

```text
move
aim
attack
dash
interact
stop
```

For movement and aim, the client sends canonical world direction after local-to-world transform.

## Latency measurement

Minimal model:

1. Client sends command with `clientSeq` and `clientTimeMs`.
2. Server replies with ack containing `clientSeq`, server tick, and status.
3. Client estimates round-trip time.

For load tests, the load client records command sent time and ack received time.

## Snapshot rate

The server tick rate and snapshot broadcast rate do not have to be identical.

Initial settings:

- tick rate: 20 Hz;
- snapshot rate: 10-20 Hz;
- metrics interval: 1 second.

If server load grows, snapshots can be coalesced.

## Client rendering

Qt and Telegram clients should render latest known snapshot. They may interpolate positions later, but MVP can render authoritative positions directly.

## Objective state in snapshots

Snapshots must include enough objective state for UI clarity:

- objective state: center/carried/dropped/respawning;
- objective canonical position;
- carrier id, if any;
- pickup lock remaining, if useful for HUD;
- score;
- base zones or static map description.

Events should include:

- objective picked up;
- objective dropped;
- objective captured;
- pickup rejected due to lock;
- carrier hit;
- match ended.

## Handling packet boundaries

TCP is a byte stream. The protocol layer must not assume one socket read equals one message.

Required decoder cases:

- one frame split across multiple reads;
- multiple frames in one read;
- partial header;
- partial payload;
- oversized declared payload;
- invalid UTF-8/JSON payload.

## Slow clients

A slow client must not cause unbounded server memory growth.

Policy:

- queue critical messages;
- coalesce snapshots;
- enforce max pending write bytes;
- disconnect repeatedly slow clients.

## Reconnect

MVP may treat reconnect as a new client. Stretch goal: allow reconnect by match token with timeout. Do not implement insecure reconnect tokens without review.
