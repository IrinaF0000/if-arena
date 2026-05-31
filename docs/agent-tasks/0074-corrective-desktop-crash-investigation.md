# Task 0074: Corrective desktop crash investigation

## Goal

Investigate the reported desktop AV-like crash surface from a clean process state and record the current reproducibility and Qt validation status before moving to SVG/crow/overlay corrective work.

## Scope

Allowed files:

- `docs/agent-runs/0074-corrective-desktop-crash-investigation.md`
- `docs/agent-tasks/0074-corrective-desktop-crash-investigation.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source code
- frontend source code
- protocol/schema changes
- scenario config changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Start desktop launcher from a clean state.
- Confirm the launcher starts the expected server/client processes without extra Qt `cmd /k` windows.
- Stop all started processes with `scripts\run\stop_if_arena.cmd`.
- Run Qt-enabled build and Qt client tests.
- Record whether the AV-like crash was reproduced.
- Do not claim full manual crash acceptance; that remains part of the final visual/manual checklist.

## Required quality gates

- Gate H. Documentation-only changes
- Gate K. Sequential agent pipeline
- Qt review for desktop launcher/test evidence

## Required checks

- `cmd /c scripts\run\stop_if_arena.cmd`
- `cmd /c "echo. | play_if_arena.cmd"`
- process inspection for `battle_server_app.exe` and `battle_qt_client.exe`
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=<local Qt path>`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure -R battle_qt_client`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: no
- Security-Agent: no
- Performance-Agent: no
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Desktop launcher smoke starts one server and two responding Qt clients from clean state.
- Stop script shuts down the started processes.
- Qt client build/tests pass.
- Crash status is recorded honestly as reproduced or not reproduced.

## Rollback note

Revert this task commit to remove the desktop crash investigation record only.
