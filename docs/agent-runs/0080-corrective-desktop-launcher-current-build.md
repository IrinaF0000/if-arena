# Run 0080: Corrective desktop launcher current build

## Status

State: committed
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0080-corrective-desktop-launcher-current-build.md`
Implementation commit: `e6239e6`

## Scope

- Desktop launcher binary selection and task/run documentation.
- No source gameplay/client behavior, workflow, deploy, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Test Impact Matrix

Changed behavior:
- `play_if_arena.cmd` now prefers `build\battle_server_app.exe` and `build\battle_qt_client.exe`.
- `build-qt-mingw` is only used as a fallback when the current `build` executable is missing.
- The launcher prints the selected Qt client executable before starting clients.

Tests added/updated:
- Happy path: launcher smoke started one server and two Qt clients from `build\`.
- Corner cases: fallback branch remains for `build-qt-mingw` if `build` binaries are missing.
- Invalid input / hostile input: not applicable; no parser/auth/network behavior changed.
- Authority / ownership: no gameplay authority behavior changed.
- Resource bounds / performance: no runtime loop or queue behavior changed.
- Regression: stop script cleaned up all launched PIDs; agent validators, secret scan, and diff check passed.
- Manual UI checks: process inspection confirmed actual executable paths.

Not tested and why:
- Full desktop click-through remains outside this narrow launcher fix.

## Checks

- `cmd /c scripts\run\stop_if_arena.cmd` - passed before launcher smoke.
- `cmd /c "echo. | play_if_arena.cmd"` - passed with escalation.
- `Get-Process battle_server_app,battle_qt_client | Select ProcessName,Id,Path,Responding` - confirmed server and both Qt clients ran from `build\`.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed after launcher smoke and stopped all three PID-file processes.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warning only.

## Reviews

- Verification-Agent: approved; launcher smoke proved current `build` executable selection.

## Risks

- If a user intentionally wants the separate `build-qt-mingw` tree, they must remove or rename the `build` executable or run that executable directly.
