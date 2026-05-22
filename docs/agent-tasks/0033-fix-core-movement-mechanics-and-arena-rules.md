# Task 0033: Fix core movement mechanics and playable arena rules

## Goal

Fix the current playable gameplay mechanics so IF Arena reads as a bottom-to-top objective arena. Do not add unrelated features. Do not create a release tag.

## Scope

Allowed files:

- `src/battle_core/**`
- `src/battle_backend/**`
- `src/battle_server_app/**` only if required for playable tick/config behavior
- `src/battle_qt_client/**`
- `frontend/telegram_mini_app/**`
- `tests/**`
- `docs/game/**`
- `docs/agent-runs/**`
- `docs/agent-tasks/0033-fix-core-movement-mechanics-and-arena-rules.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- `external/**`
- release tag creation
- task 0034 or later
- unrelated gameplay, networking, deployment, or UI features
- client-side authoritative gameplay rules

## Required behavior

- Players move with configured one-step/smooth speed and do not skip two logical cells per command/tick.
- Blue and Red can move directly forward from spawn/base toward the center objective.
- Carrier can move directly backward from center toward own base and capture automatically.
- Movement remains screen-consistent for the local player.
- Desktop Qt: WASD/arrows move; mouse movement aims only; left click/Space attack; right click/Shift dash; Space never disconnects.
- Web / Telegram: arrow buttons move; attack/dash use last aim/movement direction; stop stops movement; no idle/no-op spam.
- Objective pickup is automatic on contact. Capture is automatic at own base.
- Objective pickup/carry/capture/drop feedback is visible enough in clients and/or event output.
- Default playable arena remains bottom-to-top and includes config-driven contest pressure with obstacles and/or hazards.
- Server remains authoritative.

## Required tests

- Movement from Blue spawn directly toward objective.
- Movement from Red spawn directly toward objective.
- Movement back toward own base.
- Movement does not skip two cells/steps per command/tick.
- Local-to-canonical direction transform for both teams.
- Space maps to attack and does not disconnect.
- Attack and dash envelopes/commands are valid.
- Objective pickup on contact.
- Capture at own base.
- Default playable scenario contains expected obstacles/hazards or contest objects.
- Existing CMake, Qt, TCP, WebSocket, load, security, frontend, and agent validators still pass.

## Manual checklist

Record status in the run note:

1. Start server and two Qt clients.
2. Create/join one match.
3. From spawn, press forward/up. Player moves directly toward objective.
4. Press backward/down. Player moves directly toward own base.
5. Movement does not jump two cells.
6. Space attacks and does not disconnect.
7. LMB attacks.
8. Shift/RMB dash.
9. Objective pickup is visible.
10. Carrier state is visible.
11. Capture is visible.
12. Default map contains obstacles/hazards/contest pressure.
13. Arena is larger/readable enough.
14. No unexpected disconnects.

## Pipeline

Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.

Required reviews:

- Architecture-Agent for core/backend/client boundary changes.
- Security-Agent for protocol/network input and client authority.
- Performance-Agent for tick/movement/hazard behavior.
- Qt-Agent for desktop controls and rendering.
- Frontend-Agent for Telegram Mini App controls/rendering.

## Rollback note

One scoped task commit. Do not create a release tag.
