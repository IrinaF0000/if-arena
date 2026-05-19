# Project Map

## Core rule

The project has one authoritative backend and multiple clients. All game rules, validation, state transitions, and match outcomes live on the server side.

## Root modules

- `external/battle_simulation_snapshot/` - empty place for a copied snapshot of the original local battle simulation. Treat as read-only reference.
- `src/battle_core/` - extracted simulation/gameplay static library. No networking, Qt, Telegram, process-level server code, or deployment code.
- `src/battle_protocol/` - shared transport-independent protocol DTOs, protocol limits, serializers, parsers, and schema validation helpers.
- `src/battle_backend/` - authoritative backend application layer. Owns player sessions, match manager, match workers, command validation, rate limiting, backpressure, and metrics.
- `src/battle_transport_tcp/` - raw TCP endpoint implementation. Converts length-prefixed TCP frames into protocol messages and forwards them to `battle_backend`.
- `src/battle_transport_ws/` - WebSocket endpoint implementation. Converts WebSocket messages into protocol messages and forwards them to `battle_backend`.
- `src/battle_server_app/` - server executable composition root. Starts backend, TCP transport, WebSocket transport, metrics, and graceful shutdown.
- `src/battle_qt_client/` - Qt Widgets client. Uses raw TCP. Does not link to `battle_core`.
- `src/battle_cli_client/` - debug CLI client. Uses raw TCP.
- `tools/load_client/` - load testing client. Uses raw TCP and later WebSocket scenarios.
- `frontend/telegram_mini_app/` - TypeScript Telegram Mini App. Uses WebSocket over WSS. Does not contain authoritative game rules.
- `docs/` - product, architecture, protocol, security, performance, operations, game design, and agent workflow documentation.
- `docs/game/MAP_AND_FAIRNESS.md` - canonical MVP map size, symmetry, and grid/smooth-movement rules.
- `docs/game/OBJECTIVE_RULES.md` - Objective Run pickup, carrying, drop, pickup-lock, and capture rules.
- `docs/game/PLAYER_ORIENTED_VIEW.md` - client-side coordinate transforms and local view requirements.

## Dependency direction

Allowed:

```text
battle_server_app -> battle_backend -> battle_core
battle_server_app -> battle_transport_tcp -> battle_protocol
battle_server_app -> battle_transport_ws -> battle_protocol
battle_backend -> battle_protocol
battle_backend -> battle_core
battle_qt_client -> battle_protocol
battle_cli_client -> battle_protocol
battle_load_client -> battle_protocol
telegram_mini_app -> generated/lightweight protocol schema, not C++ core
```

Forbidden:

```text
battle_core -> TCP / WebSocket / Qt / Telegram / deployment
battle_protocol -> battle_backend / battle_server_app
Qt client -> battle_core
Telegram Mini App -> battle_core
transport layer -> game rules
client -> trusted game state authority
```

## Data flow

```text
Qt input or Telegram input
    -> transport frame
    -> protocol message validation
    -> authenticated session
    -> command queue
    -> match worker
    -> battle_core tick/apply command
    -> events/snapshots
    -> backend fanout
    -> TCP/WebSocket transport
    -> client rendering
```

## Agent rule

When changing a module, update the module's `README.md` and check this project map. If a new dependency crosses a forbidden direction, stop and ask for Architecture-Agent review.

## CI/CD

- `.github/workflows/pr-ci.yml`: validation-only pull request checks.
- `.github/workflows/main-ci.yml`: merged-tree checks and packaging placeholder.
- `docs/ci/CI_CD_GUARDRAILS.md`: mandatory rules for workflow changes and CI/CD preservation.
- `scripts/ci/validate_structure.sh`: lightweight repository structure check used by CI.


## Agent Harness

- `AGENTS.md` - root routing instructions for coding agents.
- nested `AGENTS.md` files - scoped rules for subsystems.
- `.agents/skills/` - repo-local skills loaded for specific task types.
- `docs/agent-rules/` - focused canonical rules for process, quality, security, and scalability.
- `docs/agent-manager/` - role and maintenance process for the agent harness.
- `scripts/agent/` - advisory validation and run-summary helpers.
- `docs/agent-runs/` - compact coordination plans, reviews, and retrospectives.
