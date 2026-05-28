# Subsystem Boundaries

IF Arena has one authoritative backend and multiple clients. Server authority, deterministic simulation, protocol validation, transport framing, app wiring, and rendering are separate responsibilities.

## Ownership

| Subsystem | Owns | Must not own |
| --- | --- | --- |
| `battle_core` | deterministic match simulation, movement/collision, objective rules, scoring, combat/contest rules, hazards as pure simulation, geometry and tick types, validation/use of already-parsed scenario values | protocol DTOs, backend sessions, transports, Qt, Telegram, sockets, filesystem/config parsing, environment variables, server app wiring, normal default scenario file selection |
| `battle_protocol` | envelope schema, message types, payload validation, serialization/deserialization helpers, protocol error codes | gameplay movement, pickup/capture rules, score, socket handling, Qt/Web UI behavior |
| `battle_backend` | sessions, auth state, match create/join, command queues, rate limits, ownership checks, conversion from app-loaded scenario config into core values, calls into `battle_core`, snapshot/event fanout | raw socket details, Qt/Web rendering, visual-only gameplay, client-specific shortcuts, filesystem policy |
| `battle_transport_tcp` | TCP accept/read/write, length-prefixed framing, transport close reasons, backpressure handoff | gameplay rules, movement transforms, objective logic, match scoring, UI orientation |
| `battle_transport_ws` | WebSocket frames, message limits, keepalive, transport close reasons, backend session adapter | Telegram auth authority, gameplay rules, score, UI behavior |
| `battle_server_app` | config/env reading, scenario file selection/loading, logging/metrics setup, backend and transport construction, process startup/shutdown | gameplay rules, protocol schema validation logic, client-specific behavior |
| `battle_qt_client` | rendering snapshots, keyboard/mouse input, local aim preview, connection and error UI | authoritative hit, pickup, capture, score, hazard damage, hidden simulation state |
| `frontend/telegram_mini_app` | rendering snapshots, touch/browser input, Telegram raw `initData` collection, reconnect UX | authoritative Telegram auth result, gameplay rules, secrets, score or match-result decisions |
| Assets | rendering data only | gameplay definitions, script execution, external resource loading |

## Dependency Direction

Allowed dependencies follow `PROJECT_MAP.md`:

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

Forbidden dependencies include:

```text
battle_core -> protocol/backend/transport/client/UI/deployment/IO
battle_protocol -> backend/server app/transports/clients
Qt client -> battle_core or backend internals
Telegram Mini App -> battle_core or backend internals
transport layer -> gameplay rules or UI/client headers
clients -> authoritative game state
```

## Review Rule

Any task that changes public DTO/API boundaries, CMake target dependencies, auth/session ownership, command ownership, match lifecycle, or a cross-subsystem include must receive Architecture Review Agent approval before commit.

## Scenario Ownership

Playable game scenarios are config-owned. The normal Objective Run arena is loaded from `config/scenarios/arena_small_objective_run.json`; test-only gameplay routes and expected event sequences live under `tests/scenarios/`.

Code and tests may name a scenario file or scenario id, but must not duplicate map layouts, hazard metadata, movement routes, or expected objective event sequences. Scenario validation and anti-hardcoding checks are part of the playable gate:

```bash
python scripts/ci/validate_no_hardcoded_scenarios.py
python scripts/ci/validate_gameplay_scenario_pairs.py
```

## Validation

`scripts/ci/validate_architecture_boundaries.py` is the lightweight automated guard. It checks forbidden include patterns, selected target dependencies, client/transport include boundaries, and production SVG safety. The validator is intentionally conservative and should be extended with narrow checks rather than broad rewrites.
