# Playable Game Agent Plan

This plan coordinates tasks `0016` through `0028` for turning the IF Arena foundation into a playable two-player Objective Run demo. Work must follow `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.

## Completion Scenarios

### Scenario A: Deterministic Core Match

Tasks: `0017`, `0018`, `0019`, `0020`.

The core simulation can run a complete two-player Objective Run match with a 21x13 symmetric map, movement/collision, objective pickup/carry/drop/capture, combat, dash, hazards, scoring, and deterministic tests.

### Scenario B: Backend In-Process Match

Tasks: `0021`, `0022`, `0023`.

The backend can run one authoritative local match loop with fake sessions, validated playable protocol messages, snapshots/events, and a server app configuration path that is honest about local-only readiness.

### Scenario C: CLI Raw TCP Playable Flow

Tasks: `0024`, `0025`.

Two local CLI or TCP clients can connect through the raw TCP path, send intentions only, and complete a locally testable Objective Run flow against the authoritative server.

### Scenario D: WebSocket And Telegram Mini App Slice

Task: `0026`.

The WebSocket path and Telegram Mini App can run a local playable slice with runtime-validated inbound messages, backend-owned Telegram auth validation, and no frontend authority over game state.

### Scenario E: Qt Playable Client

Task: `0027`.

The Qt client can connect over TCP, render the playable arena, send intentions, display connection/errors, and preserve server authority.

## Release Hardening

Task: `0028`.

Finish with load/security hardening, reproducible demo instructions, honest limitations, and portfolio-ready documentation. Any scenario that remains incomplete must be explicitly marked as later stretch scope in README and supporting docs.

## Task Order

1. `0016` clean foundation before playable slice.
2. `0017` add core arena map and config.
3. `0018` add core player movement and collision.
4. `0019` add Objective Run rules.
5. `0020` add combat, dash, and hazards.
6. `0021` add playable gameplay protocol messages.
7. `0022` add backend match loop and fake sessions.
8. `0023` add server app and local config.
9. `0024` add CLI two-player playable flow.
10. `0025` add TCP vertical slice integration.
11. `0026` add WebSocket and Telegram playable slice.
12. `0027` complete Qt playable client.
13. `0028` complete load, security hardening, and portfolio docs.
