# Task 0072: Corrective run/stop hygiene

## Goal

Add reliable local stop/start hygiene for corrective manual testing so stale IF Arena server/client/Vite processes do not interfere with desktop or mobile validation.

## Scope

Allowed files:

- `scripts/run/stop_if_arena.cmd`
- `play_if_arena.cmd`
- `play_if_arena_ws.cmd`
- `scripts/README.md`
- `README.md`
- `docs/agent-runs/0072-corrective-run-stop-hygiene.md`
- `docs/agent-tasks/0072-corrective-run-stop-hygiene.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source gameplay/client code
- protocol/schema changes
- scenario config changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Add `scripts/run/stop_if_arena.cmd`.
- Stop `battle_server_app.exe` and `battle_qt_client.exe`.
- Stop IF Arena port owners for `5555`, `8081`, and `5173`.
- Stop `node.exe` only when it owns a known IF Arena port; never kill all node processes globally.
- Support `--keep-vite` to preserve Vite on `5173`.
- Launchers call the stop script at startup.
- Qt launcher starts Qt GUI clients without extra `cmd /k` terminal windows.
- Launchers write logs/PID files under `build/run-logs/` and `build/run-state/`.
- Documentation explains start/stop behavior and ports.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Script/process safety review

## Required checks

- `cmd /c scripts\run\stop_if_arena.cmd --keep-vite`
- `cmd /c scripts\run\stop_if_arena.cmd`
- `git diff --check`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`

## Required reviews

- Architecture-Agent: no
- Security-Agent: yes
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Running the stop script succeeds with and without `--keep-vite`.
- Launchers no longer use `cmd /k` for Qt GUI clients.
- Stop script does not kill unrelated browser processes or all `node.exe` processes globally.
- No source, CI/CD, deploy, generated, protocol, scenario config, push, tag, or merge changes are made.

## Rollback note

Revert this task commit to return local launcher behavior to the previous manual stop process.
