# Task 0065: Desktop side panel layout

## Goal

Move Qt desktop service UI into a stable right side panel and keep the arena focused on gameplay visuals.

## Scope

Allowed files:

- `src/battle_qt_client/ui/MainWindow.*`
- `src/battle_qt_client/ui/ArenaView.*`
- `src/battle_qt_client/ui/README.md`
- `tests/manual/qt_client_checklist.md`
- `docs/agent-runs/0065-desktop-side-panel-layout.md`
- `docs/agent-tasks/0065-desktop-side-panel-layout.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay rules
- protocol/schema changes
- scenario config changes
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Qt desktop uses a main arena area and a stable right side panel.
- Connection, lobby, status, HUD, controls hint, errors, next-match action, and event log live in the side panel.
- Arena overlays remain short gameplay surfaces only.
- Desktop base visuals are compact zone markers rather than large full-width bands.
- No client-side authority or network/session ownership changes.

## Required quality gates

- Gate A. Every implementation task
- Gate D. Qt client
- Gate K. Sequential agent pipeline
- Gate L. Playable-game scenario integrity

## Required checks

- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/desktop/objective_run_full_capture_desktop.py`
- `python tests/integration/desktop/objective_event_sequence_desktop.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Desktop screenshot/layout shows a clean arena with service UI in the right side panel.
- The event log scrolls in the panel and does not compress the arena vertically.
- Base visuals are smaller while capture/scoring remains server-owned.
- Existing Qt build and desktop gameplay scenario checks pass.

## Rollback note

Revert this task commit to return the Qt client to the previous vertical layout.
