# IF Arena

IF Arena is a small real-time 2-player arena game built as a C++20 networking, backend, Qt, Telegram Mini App, security, and agentic-development portfolio project.

The name combines the author's initials, `IF`, with a small programming pun. The project name is intentionally independent of any commercial game platform or brand.

## Product concept

Two players join a short real-time objective arena match. Each player controls one hero. The arena is authored on a logical grid but movement is smooth and real-time.

The MVP mode is **Objective Run**:

- two players spawn on opposite sides of a compact arena;
- a objective starts at the exact center;
- players compete to pick it up and carry it back to their own base;
- the first player to score 3 captures wins;
- the carrier moves slower;
- the carrier drops the objective when hit;
- after a drop, a short pickup lock prevents instant re-pickup;
- neutral server-controlled hazards such as mines, towers, and drones/crows create tactical pressure.

The clients do not own game state. They send player intentions such as move, aim, attack, dash, and interact. The server validates every command and applies accepted commands to the authoritative simulation.

## MVP arena requirements

- Recommended MVP map size: **21 x 13 logical cells**.
- One center cell contains the initial objective spawn.
- The arena uses **180-degree rotational symmetry** around the central objective.
- Both players have equal stats and equal start conditions.
- Bases, spawn points, obstacles, hazards, and route options are symmetric.
- The whole arena should fit on screen in the MVP.
- Player movement is smooth; the grid is mostly a level-authoring and collision structure.
- The floor may show a subtle tile/grid pattern, but the game should feel like a top-down real-time arena, not a strict turn-based board game.

## Player-oriented view

The server uses one canonical coordinate system. Clients may render a local player-oriented view:

- each player sees their own base at the bottom of the screen;
- the enemy base appears at the top;
- `W` or forward touch input always means moving toward the center/enemy side in the local view;
- local controls are interpreted in player view and the local TCP backend transforms team-local command directions to canonical world movement;
- replay, spectator, and debug views use canonical orientation.

Player view may use local colors: own hero blue/cyan, enemy hero red. Replay/debug views may use fixed team colors.

## One backend, two UI clients

```text
                    +------------------------+
                    |  battle_server_app     |
                    |  one backend process   |
                    +-----------+------------+
                                |
              +-----------------+-----------------+
              |                                   |
      raw TCP transport                    WebSocket transport
              |                                   |
      Qt desktop client                 Telegram Mini App
      CLI/load clients                   browser/WebView client
```

The Qt client demonstrates C++ desktop UI, Qt networking integration, and low-latency keyboard/mouse gameplay over raw TCP.

The Telegram Mini App demonstrates a lightweight mobile/web UX with invite-based play. Browser-based Mini Apps cannot use raw TCP directly, so they connect through WebSocket. The backend game logic and protocol DTOs remain shared.

## Repository layout

```text
external/battle_simulation_snapshot/
  Empty place for a read-only copy of the original local simulation project.

src/battle_core/
  Static library extracted from the old simulation snapshot.

src/battle_protocol/
  Transport-independent DTOs, schema validation helpers, protocol limits.

src/battle_backend/
  Authoritative application layer: sessions, match manager, workers, command routing,
  metrics, rate limiting, and backpressure policies.

src/battle_transport_tcp/
  Raw TCP transport for Qt, CLI, and load clients.

src/battle_transport_ws/
  WebSocket gateway for Telegram Mini App clients.

src/battle_server_app/
  Executable that starts the backend and enabled transports.

src/battle_qt_client/
  Qt Widgets desktop client.

frontend/telegram_mini_app/
  TypeScript Telegram Mini App frontend.

tools/load_client/
  Simulated client load testing tool.

docs/
  Product, architecture, protocol, security, performance, agent workflow, and game design docs.
```

## Development principles

- `battle_core` must not depend on TCP, WebSocket, Qt, Telegram, deployment, or process-level server code.
- `battle_protocol` must remain transport-independent.
- `battle_backend` owns all game authority, validation, session state, match workers, metrics, and resource limits.
- Transports are adapters, not game-rule owners.
- Qt and Telegram clients are presentation/input layers only.
- Every network input is untrusted.
- Every resource that can grow must have a bound.
- Secrets must not be committed, logged, or embedded in frontend code.
- Agentic implementation must be done in small, reviewable tasks with explicit quality gates.

## First milestones

1. Copy the old project into `external/battle_simulation_snapshot/`.
2. Extract a minimal `battle_core` static library without changing the old repository.
3. Add transport-independent protocol DTOs and validation.
4. Build a headless backend with one local Objective Run match and deterministic tick loop.
5. Add raw TCP transport and CLI client.
6. Add Qt desktop client with player-oriented rendering.
7. Add WebSocket transport and Telegram Mini App.
8. Add load testing, metrics, security hardening, and deployment docs.

## Status

This repository has the foundation modules, an in-process backend match loop, local raw TCP and WebSocket vertical slices, a Telegram Mini App slice, and a Qt Widgets playable client target. Local CLI/TCP, Qt, and Telegram/WebSocket clients can create/join a demo Objective Run match, send intention-only commands, and receive authoritative snapshots/events from the server.

Use `scripts/run_local_server.sh` to build and start the current `battle_server_app` config/backend slice. Public deployment is not ready until the later load and security-hardening slice is complete.

Local raw TCP smoke:

```bash
cmake --build build --parallel
build/battle_server_app --config config/examples/server.local.json --max-clients 2
build/battle_cli_client --create --display-name cli-one --script tests/integration/server/cli_idle.script
build/battle_cli_client --join M1 --display-name cli-two --script tests/integration/server/cli_scenario_b.script
```

For repeatable hostile-input coverage after building, run:

```bash
python tests/integration/server/tcp_vertical_slice_smoke.py
python tests/frontend/websocket_local_smoke.py
```

Telegram Mini App frontend checks:

```bash
cd frontend/telegram_mini_app
npm run typecheck
npm run lint
npm run build
```

Qt desktop client with a local Qt install:

```bash
cmake -S . -B build-qt -DBATTLE_BUILD_QT_CLIENT=ON -DBATTLE_BUILD_TESTS=ON
cmake --build build-qt --parallel
build-qt/battle_qt_client
```

## CI/CD safety

PR and main workflows are intentionally separated. PR CI validates C++, TypeScript, repository structure, and secret-scanning without deployment or production secrets. Main CI repeats validation on the merged tree and may build non-sensitive artifacts. Workflow changes are protected by `docs/ci/CI_CD_GUARDRAILS.md`.

## Agent-oriented development harness

This repository includes a scoped agent harness for Codex-style parallel development:

- root and nested `AGENTS.md` files;
- repo-local skills in `.agents/skills/`;
- focused rules in `docs/agent-rules/`;
- task packets in `docs/agent-tasks/`;
- Agent Manager docs in `docs/agent-manager/`;
- scripts for structure, harness, and secret validation.

The harness is designed to keep security, code quality, access boundaries, role separation, scalability, CI/CD safety, and token economy explicit during agent-driven implementation.
