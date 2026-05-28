# Task 0054: Desktop and mobile visual polish

## Goal

Polish the desktop Qt and Telegram Mini App arena presentation so screenshots/GIFs clearly show players, bases, objective, hazards, and event state.

## Scope

Allowed files:

- `frontend/telegram_mini_app/src/**`
- `src/battle_qt_client/ui/**`
- `tests/manual/qt_client_checklist.md`
- `docs/game/VISUAL_READABILITY.md`
- `docs/agent-runs/0054-desktop-and-mobile-visual-polish.md`
- `docs/agent-tasks/0054-desktop-and-mobile-visual-polish.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay rules
- protocol schemas
- scenario configs
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Increase player visual token size without changing gameplay collision/radius.
- Modernize board, base, obstacle, objective, hazard, and HUD colors in both clients.
- Keep the whole 21x13 arena visible.
- Preserve player-oriented view rules and existing transform behavior.
- Keep all client changes presentation-only.

## Required quality gates

- Gate A. Every implementation task
- Gate D. Qt client
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/mobile/objective_run_full_capture_mobile.py`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python tests/integration/mobile/objective_event_sequence_mobile.py`
- `python scripts/ci/validate_gameplay_scenario_pairs.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- Qt MinGW configure/build/CTest when Qt is available
- Browser visual smoke screenshot for Telegram Mini App when dev server can run
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Important objects are readable immediately in both clients.
- Player visual size is larger while gameplay state remains server-owned.
- Existing transform, scenario, and frontend checks still pass.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to return to the previous visual style.
