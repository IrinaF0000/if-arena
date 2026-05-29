# Task 0058: Desktop and mobile scenario consistency

## Goal

Make desktop and mobile clients observe the same authoritative scenario metadata and prevent hidden client-specific scenario drift.

## Scope

Allowed files:

- `src/battle_backend/**`
- `src/battle_server_app/**`
- `src/battle_qt_client/**`
- `frontend/telegram_mini_app/src/**`
- `tests/unit/**`
- `tests/frontend/**`
- `tests/integration/**`
- `docs/architecture/PROTOCOL.md`
- `docs/agent-runs/0058-desktop-mobile-scenario-consistency.md`
- `docs/agent-tasks/0058-desktop-mobile-scenario-consistency.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- scenario configs
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Server-origin `match_joined` and `snapshot` payloads expose scenario metadata.
- Desktop and mobile clients parse and display the scenario id in service/debug UI.
- Desktop and mobile scenario tests assert the same scenario id and authoritative map/object metadata.
- Clients continue to send intentions only and do not own gameplay rules.

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
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `node tests/frontend/telegram_websocket_client_behavior.mjs`
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
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Desktop and mobile wrappers fail if scenario metadata is absent or inconsistent.
- Both clients show scenario id outside the arena.
- No source of scenario truth is added to client code.
- No push, tag, or merge is performed.

## Rollback note

Revert this task commit to remove scenario metadata propagation and client display changes.
