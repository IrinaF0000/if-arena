# Run 0074: Corrective desktop crash investigation

## Status

State: commit-ready
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0074-corrective-desktop-crash-investigation.md`
Implementation commit: pending

## Scope

- Documentation-only investigation record.
- No source, workflow, deploy, protocol/schema, scenario config, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: not applicable; investigation record only.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- The previously reported white/AV-like crash may have been caused or amplified by stale server/client process state. Full manual crash acceptance still requires the later final manual flow.

## Test Impact Matrix

Changed behavior:
- Documentation-only investigation record; no runtime behavior changes.

Tests added/updated:
- Happy path: clean desktop launcher smoke started one server and two Qt clients.
- Corner cases: stop script was run before and after launcher smoke.
- Invalid input / hostile input: not applicable; no parser/auth/protocol behavior changed.
- Authority / ownership: no gameplay authority behavior changed.
- Resource bounds / performance: launcher smoke confirmed expected process count after Phase 1 window/process hygiene.
- Regression: Qt-enabled build, Qt client tests, agent validators, secret scan, and diff check are required.
- Manual UI checks: launcher process smoke was run; full desktop play/rematch/disconnect visual flow remains pending for final manual acceptance.

Not tested and why:
- The full connect/create/join/play/score/rematch/disconnect/close desktop flow was not completed in this investigation checkpoint; it belongs to the final corrective manual acceptance after SVG/crow/overlay fixes.

## Checks

- `cmd /c scripts\run\stop_if_arena.cmd` - passed before launcher smoke.
- `cmd /c "echo. | play_if_arena.cmd"` - passed; launcher returned after stdin satisfied the final pause.
- Process inspection - passed; observed one `battle_server_app.exe` and two responding `battle_qt_client.exe` processes.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed after launcher smoke; stopped both Qt clients and the server through PID files.
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64` - passed with optional Vulkan header warning only.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure -R battle_qt_client` - passed, 2/2 tests.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed.

## Reviews

- Verification-Agent: approved; launcher/process and Qt build/test evidence recorded.
- Qt-Agent: approved for checkpoint scope; no AV-like crash reproduced during clean launcher startup, but full manual flow remains pending.

## Risks

- Crash not reproduced in this checkpoint. Do not treat this as final desktop stability acceptance until the corrective final manual flow is completed.
