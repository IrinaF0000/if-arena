# ADR 0001: Use an Authoritative Server

## Status

Accepted.

## Context

The project adds multiplayer networking to a battle simulation. Remote clients should control units through TCP. A key architectural decision is whether clients are allowed to own game state or whether the server is the single source of truth.

## Decision

The server is authoritative.

Clients send player intentions:

- movement input
- attack request
- join/create match requests

The server owns:

- match state
- unit positions
- HP
- cooldowns
- neutral hazards
- validation rules
- win conditions
- replay input log

## Consequences

### Positive

- clients cannot directly cheat by sending arbitrary state
- game rules remain centralized
- server-side replay is possible
- integration tests can validate behavior without UI
- architecture looks realistic for backend/game-server work

### Negative

- client code is slightly more complex because it must render server snapshots
- latency is more visible than with local-only simulation
- client-side prediction may be needed later for very responsive movement

## Notes

The MVP does not implement client-side prediction. It uses direct server snapshots. This is acceptable for a local demo and keeps the project simpler.
