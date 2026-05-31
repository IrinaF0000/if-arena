# Run 0072: Corrective run/stop hygiene

## Status

State: committed
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0072-corrective-run-stop-hygiene.md`
Implementation commit: `695fae5`

## Scope

- Local Windows run/stop scripts and focused documentation only.
- No source gameplay/client code, workflow, deploy, protocol/schema, scenario config, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Assumptions

- Corrective plan Phase 1 should be handled before mobile protocol, crash, visual, and crow fixes.
- Existing `.cmd` launchers are the relevant manual desktop/mobile entry points.

## Test Impact Matrix

Changed behavior:
- Adds a safe stop script and makes local launchers start from a clean process state with logs/PID files.

Tests added/updated:
- Happy path: run the stop script with and without `--keep-vite`.
- Corner cases: `--keep-vite` leaves Vite out of the port stop list.
- Invalid input / hostile input: no network parser/auth/protocol behavior changed.
- Authority / ownership: no gameplay authority behavior changed.
- Resource bounds / performance: launchers no longer leave extra Qt `cmd /k` windows; PID/log directories are bounded to local run files.
- Regression: agent validators, secret scan, and diff check are required.
- Manual UI checks: full launcher/manual UI acceptance remains later in the corrective plan after mobile and desktop stability fixes.

Not tested and why:
- Full desktop/mobile manual launcher flow is deferred until later corrective phases because Phase 1 only establishes the process hygiene tools.

## Checks

- `cmd /c scripts\run\stop_if_arena.cmd --keep-vite` - passed.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed with line-ending normalization warnings only.

## Reviews

- Verification-Agent: approved; required checks passed and diff stayed within allowed files.
- Security-Agent: approved; stop script does not kill browsers and only stops `node.exe` when it owns a known IF Arena port or tracked PID file process.

## Risks

- `npm.cmd` may spawn a child Node/Vite process while the PID file records the parent process; the stop script also stops the known Vite port owner to cover that case.
- Full manual launcher acceptance is intentionally deferred to later corrective phases after mobile protocol and desktop stability fixes.
