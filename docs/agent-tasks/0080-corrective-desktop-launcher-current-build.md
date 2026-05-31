# Task 0080: Corrective desktop launcher current build

## Goal

Make the desktop launcher prefer the current `build` Qt client instead of silently launching stale `build-qt-mingw` binaries.

## Scope

Allowed files:

- `play_if_arena.cmd`
- `docs/agent-runs/0080-corrective-desktop-launcher-current-build.md`
- `docs/agent-tasks/0080-corrective-desktop-launcher-current-build.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay/client behavior
- generated build outputs
- generated dependency directories
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Desktop launcher uses `build\battle_qt_client.exe` when present.
- Desktop launcher falls back to `build-qt-mingw\battle_qt_client.exe` only when `build` client is missing.
- Error message points users at the preferred `build` command first.
- Launcher still calls the stop script and records PID/log files.

## Required checks

- `cmd /c scripts\run\stop_if_arena.cmd`
- `cmd /c "echo. | play_if_arena.cmd"`
- `cmd /c scripts\run\stop_if_arena.cmd`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Verification-Agent: yes
- Architecture-Agent: no
- Security-Agent: no
- Performance-Agent: no

## Acceptance criteria

- Launcher starts the preferred current-build Qt client path when it exists.
- Stop script cleans up server/client processes after launcher smoke.

## Rollback note

Revert this task commit to restore the previous `build-qt-mingw`-first desktop launcher behavior.
