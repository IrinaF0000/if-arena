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

## Architecture changes

Any public API, threading model, protocol schema, auth model, or deployment exposure change requires Architecture and Security review.
