# battle_core Boundaries

`battle_core` is pure deterministic simulation. It must be reusable in tests and tools without sockets, Qt, Telegram, environment variables, filesystem config loading, or server process wiring.

## Internal Layers

### Minimal Foundation

Owns neutral primitives:

- ids;
- vectors and geometry;
- tick types;
- deterministic helpers;
- small value/result types.

Must not know about Objective Run, Telegram, Qt, scenario files, network sessions, player colors, or default maps.

### Gameplay Features

Owns pure deterministic game rules:

- movement and collision;
- objective pickup, drop, carrier slow, capture, and score;
- attack/contest rules;
- dash;
- hazards.

Feature code may know game-specific concepts, but it still must not know transport, UI, config-file parsing, or server sessions.

### Scenario Value Config

Owns deterministic validation and use of already-parsed arena values:

- map dimensions and geometry;
- base/spawn/objective placement values;
- obstacle and hazard values;
- scenario validation;
- gameplay rule parameters such as objective, combat, movement, and hazard settings.

Must not own filesystem paths, JSON parsing, or normal default scenario construction. The playable default scenario lives in `config/scenarios/arena_small_objective_run.json` and is parsed outside `battle_core` before value-type config reaches the simulation.

### View/Input Transforms

Player-oriented view/input transforms are pure helpers, not gameplay truth. They may convert between local player view and canonical world coordinates, but must not decide pickup, capture, hit, damage, cooldown, score, or match result.

## Temporary MVP Exceptions

These exceptions are tolerated only until a dedicated refactor splits them cleanly:

- `ArenaConfig.hpp` may contain legacy/default Objective Run helpers for focused tests or explicit fallback paths only.
- Player-view transform helpers may live in `battle_core` while backend and clients still share the MVP direction contract.

Do not grow these exceptions. New gameplay should live in feature/scenario layers, and new IO/config parsing must stay outside `battle_core`.

## Forbidden Production Includes

Production files under `src/battle_core/**` must not include:

- `battle_protocol`, `battle_backend`, `battle_transport_tcp`, `battle_transport_ws`, or `battle_qt_client`;
- Qt headers;
- WinSock, `asio`, or socket headers;
- `<filesystem>`, `<fstream>`, or `<iostream>`;
- server app, environment, secret, or config-loading headers.

Use `scripts/ci/validate_architecture_boundaries.py` before review.
