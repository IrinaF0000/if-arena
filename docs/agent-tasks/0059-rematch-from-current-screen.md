# Task 0059: Rematch from current screen

## Goal

Allow players to start a fresh match from the current game screen after an authoritative match finish, without leaving the desktop or mobile client flow.

## Scope

Allowed files:

- `src/battle_protocol/**`
- `src/battle_backend/**`
- `src/battle_server_app/**`
- `src/battle_qt_client/**`
- `frontend/telegram_mini_app/src/**`
- `tests/unit/**`
- `tests/frontend/**`
- `tests/integration/**`
- `tests/scenarios/**`
- `docs/architecture/PROTOCOL.md`
- `docs/agent-runs/0059-rematch-from-current-screen.md`
- `docs/agent-tasks/0059-rematch-from-current-screen.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Clients can request `start_next_match` only after a server-finished match.
- The backend creates a fresh match for the same participants, same configured scenario, reset scores/objective, and reset per-session command sequence.
- Desktop and mobile clients expose a next-match control without leaving the active game screen.
- Desktop and mobile scenario wrappers cover the rematch flow.

## Required quality gates

- Gate A. Every implementation task
- Gate B. Networking and protocol
- Gate C. Backend/gameplay authority
- Gate D. Qt client
- Gate E. Telegram Mini App
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/rematch_same_screen_desktop.py`
- `python tests/integration/mobile/rematch_same_screen_mobile.py`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
- `node tests/frontend/telegram_protocol_validation.mjs`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python tests/ci/validate_architecture_boundaries_tests.py`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `npm run typecheck`
- `npm run lint`
- `npm run build`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes
- Performance-Agent: yes
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- `start_next_match` rejects unfinished matches and authority claims.
- The new match id differs from the finished match id.
- Both clients receive fresh `match_joined` and reset snapshots.
- No source of gameplay authority moves into clients.
- No push, tag, or merge is performed.

## Rollback note

Revert this task commit to remove rematch protocol, backend restart, client controls, and scenario coverage.
