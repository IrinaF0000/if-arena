# Architecture Quality Rules

## Layering

- `battle_core`: deterministic gameplay only.
- `battle_protocol`: transport-neutral message DTOs and validation.
- `battle_backend`: authoritative server orchestration, sessions, matches, metrics.
- `battle_transport_tcp`: raw TCP framing and sessions.
- `battle_transport_ws`: WebSocket gateway and Telegram session entry.
- `battle_qt_client`: desktop UI client.
- `frontend/telegram_mini_app`: web/mobile client.

## Dependency direction

Clients and transports depend on protocol. Backend depends on core and protocol. Core depends on neither transport nor UI.

Canonical boundary details live in:

- `docs/architecture/SUBSYSTEM_BOUNDARIES.md`
- `docs/architecture/BATTLE_CORE_BOUNDARIES.md`
- `PROJECT_MAP.md`

## Architecture changes

Any public API, threading model, protocol schema, auth model, or deployment exposure change requires Architecture and Security review.

## Boundary enforcement

- Run `python scripts/ci/validate_architecture_boundaries.py` for post-0034 implementation tasks.
- Treat validator failures as blocking unless the task packet explicitly updates the boundary policy and receives Architecture Review Agent approval.
- Do not grow temporary MVP exceptions in `battle_core`; extract them in a dedicated task instead.
