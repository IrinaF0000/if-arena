# Project Brief

## Project name

IF Arena

## Short description

A C++20 real-time 2-player objective arena game with an authoritative high-load-oriented backend, Qt desktop client over raw TCP, Telegram Mini App over WebSocket/WSS, neutral server-controlled hazards, deterministic replay, and load-testing tooling.

## Why this project exists

This project is a portfolio project for demonstrating:

- modern C++ backend architecture;
- raw TCP socket programming and protocol framing;
- WebSocket gateway design for browser/WebView clients;
- Qt desktop UI skills;
- strict TypeScript Telegram Mini App frontend practices;
- secure handling of hostile network input;
- high-load-oriented backend practices;
- agentic development workflows with scoped instructions, skills, task packets, and CI guardrails.

## Gameplay summary

The MVP mode is **Objective Run**.

Two players connect to the same authoritative server and fight in a compact arena. They move with fast controls, aim, attack, dash/dodge, and compete for a central objective. A player scores by carrying the objective to their own base. The first to 3 captures wins.

The carrier moves slower and drops the objective when hit. A short pickup lock prevents immediate re-pickup. Neutral towers, mines, and optional drones/crows are controlled by the server and create tactical pressure.

## Map summary

- Recommended MVP size: 21 x 13 logical cells.
- Map is authored on a grid, but player movement is smooth.
- The arena has 180-degree rotational symmetry around the central objective.
- Both players have equal stats and symmetric start conditions.
- Clients render a player-oriented view where own base is at the bottom.
- Server, replay, spectator, and debug tools use canonical coordinates.

## Architecture summary

```text
Qt client            -> raw TCP      -> transport adapter
Telegram Mini App    -> WebSocket    -> transport adapter
CLI/load clients     -> raw TCP      -> transport adapter
                                      -> shared backend sessions
                                      -> authoritative match workers
                                      -> battle_core static library
```

The original local battle simulation repository is not modified. A copy is placed under `external/battle_simulation_snapshot/` as a read-only source snapshot. Reusable gameplay logic is extracted into `src/battle_core/` as a static library.

## MVP success criteria

- One authoritative backend process can host both TCP and WebSocket clients.
- Two players can complete an Objective Run match.
- Qt and Telegram clients both use the same backend gameplay mode.
- The backend validates all commands and never trusts client state.
- Networking uses bounded queues, rate limiting, frame/message limits, and safe disconnects.
- Security rules for Telegram auth, secrets, and logging are enforced.
- Load client can simulate many clients and report metrics.
- README and docs explain architecture, setup, tests, and deployment.
