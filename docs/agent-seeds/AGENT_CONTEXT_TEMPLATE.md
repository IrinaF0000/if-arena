# Agent Context

Stable project context. Current work is tracked in `docs/agent-runs/`.

## Identity

Project: IF Arena
Purpose: C++20 networking/backend/Qt/Telegram Mini App portfolio project
Primary language: C++20
Secondary languages/tools: Qt, TypeScript, CMake, GitHub Actions
Target platforms: desktop Qt client, Telegram Mini App, Linux backend
Public portfolio goal: show secure, scalable, high-quality engineering with agentic workflows

## Product goal

Build a small real-time 2-player objective arena game with one authoritative C++20 backend, a Qt desktop client over raw TCP, and a Telegram Mini App over WebSocket/WSS.

MVP mode: Objective Run. Players compete for a central objective, carry it to their base, and win by reaching the score limit. The carrier moves slower, drops the objective on hit, and a short pickup lock prevents immediate re-pickup.

## Non-negotiable qualities

- Maintainable architecture.
- Secure handling of untrusted network input.
- Deterministic server-side simulation.
- High-load-oriented bounded resources.
- Clear ownership and lifetime management.
- Reproducible builds and tests.
- Documentation updated with behavior changes.
- LLM token economy during agent work.

## Architecture overview

- `battle_core`: deterministic simulation, rules, actions, events, snapshots. No TCP, WebSocket, Qt, Telegram, deployment, or server-process dependencies.
- `battle_backend`: authoritative sessions, match manager, workers, command validation, metrics, rate limiting, and backpressure.
- `battle_transport_tcp`: raw TCP adapter for Qt/CLI/load clients.
- `battle_transport_ws`: WebSocket adapter for Telegram Mini App clients.
- `battle_protocol`: transport-independent message definitions, limits, validation helpers, serialization.
- `battle_qt_client`: Qt Widgets client, player-oriented rendering, keyboard/mouse input, raw TCP network client wrapper.
- `frontend/telegram_mini_app`: strict TypeScript Mini App, player-oriented Canvas rendering, touch controls, WebSocket client.
- `tests`: unit, integration, protocol, fake client, load, security, and UI manual tests.
- `docs`: architecture, protocol, security, deployment, ADRs, game design, and agent harness.

## Gameplay constants to preserve until explicitly changed

- Map: 21 x 13 logical cells.
- Symmetry: 180-degree rotational symmetry around the central objective.
- View: each player sees own base at bottom; server uses canonical coordinates.
- Movement: smooth real-time movement on a grid-authored map.
- Win condition: first to 3 captures by default.
- Carrier: slower, drops objective on successful hit, pickup lock after drop.
- Hazards: server-controlled neutral mines/towers, optional drone/crow.

## Security model

- Server is authoritative.
- Clients are untrusted.
- All network input is hostile until validated.
- The client may assist UI decisions, but never owns game rules.
- Invalid commands must fail safely and must not corrupt match state.
- No secrets in source code, docs, logs, tests, screenshots, or commits.
- Telegram `initData` is validated only on the backend; `initDataUnsafe` is never trusted.

## Approved dependency categories

- Networking: Boost.Asio or standalone Asio.
- UI: Qt Widgets and Qt Network.
- WebSocket: approved C++ WebSocket library only after dependency rationale.
- JSON: a chosen JSON library with explicit exception/error handling.
- Testing: existing project framework or Catch2/GoogleTest if explicitly approved.
- Formatting/static analysis: clang-format, clang-tidy, compiler warnings, sanitizers.

Add new third-party dependencies only after updating dependency rationale and license notes.

## Baseline commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
bash scripts/ci/validate_structure.sh
python3 scripts/ci/scan_secrets.py
python3 scripts/agent/validate_agent_harness.py
git diff --check
```

## Key documents

- `README.md`
- `PROJECT_MAP.md`
- `docs/project/PRODUCT_REQUIREMENTS.md`
- `docs/architecture/TECHNICAL_ARCHITECTURE.md`
- `docs/architecture/PROTOCOL.md`
- `docs/game/GAME_DESIGN.md`
- `docs/game/MAP_AND_FAIRNESS.md`
- `docs/game/OBJECTIVE_RULES.md`
- `docs/game/PLAYER_ORIENTED_VIEW.md`
- `docs/security/SECURITY_REQUIREMENTS.md`
- `docs/review/QUALITY_GATES.md`
- `docs/agent-rules/process/TOKEN_ECONOMY.md`
