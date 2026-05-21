# Task 0031: Fix Playable Controls And Arena Readability

## State

committed

## Coordinator Preflight

- Branch: `agent/0031-fix-playable-controls-and-arena-readability`.
- Baseline: `c463067 docs: add task 0031 controls and readability plan`.
- Working tree before implementation: clean.
- Allowed implementation scope confirmed: `src/battle_qt_client/**`, `src/battle_server_app/**`, `src/battle_backend/**` if needed for tick/application after `1200`, `tests/**`, `docs/game/**`, `tests/manual/**`, `docs/agent-runs/`.
- Forbidden scope checked: no `.github/workflows/**`, `deploy/**`, `external/**`, unrelated Telegram/frontend, or release tag work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Review-Agent -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent

Assumptions:

- Qt mouse aim should remain local for this MVP because the current backend maps protocol `aim` to movement.
- The live TCP freeze is caused by the server ticking a match only on join/input; removing mouse aim command spam requires the TCP runtime to advance active matches on a timer.
- Objective pickup/capture remains authoritative and automatic.

Changed files:

- `src/battle_server_app/main.cpp`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_qt_client/network/NetworkClient.cpp`
- `src/battle_qt_client/ui/MainWindow.hpp`
- `src/battle_qt_client/ui/MainWindow.cpp`
- `src/battle_qt_client/ui/ArenaView.hpp`
- `src/battle_qt_client/ui/ArenaView.cpp`
- `tests/integration/server/tcp_live_tick_smoke.py`
- `tests/manual/qt_client_checklist.md`
- `docs/game/CONTROLS_AND_UI.md`
- `docs/game/VISUAL_READABILITY.md`
- `docs/agent-runs/0031-fix-playable-controls-and-arena-readability.md`

Implementation notes:

- Added a TCP active-match timer that advances started matches at the configured tick interval during socket poll timeouts.
- Increased the backend playable match tick cap from the core default `1200` to `3600` for this MVP scenario so matches do not appear frozen at the one-minute mark.
- Input commands now queue until the next due tick instead of forcing a match tick on every accepted input command.
- Qt mouse movement updates local aim preview only and does not emit gameplay input.
- Left click/Space send attack; right click/Shift send dash; both use aim direction with movement/default fallback.
- Duplicate movement intents and repeated idle stop intents are suppressed.
- Accepted input acknowledgements are no longer logged repeatedly; rejected input remains visible.
- The Qt arena renders a subtler grid, a narrower bottom-to-top play field, objective ring/diamond, carrier halo/marker, aim/range preview, attack/dash feedback, and HP rings instead of bars above players.

Checks run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass after stopping a stale local `build\battle_server_app.exe` process that was locking the output executable.
- `ctest --test-dir build --output-on-failure` - pass.
- `python tests\integration\server\tcp_live_tick_smoke.py` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake --build build-qt-mingw --parallel` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; ctest --test-dir build-qt-mingw --output-on-failure` - pass.
- `python tests\integration\server\tcp_vertical_slice_smoke.py` - pass.
- `python tests\load\load_client_dry_run.py` - pass.
- `python tests\load\local_tcp_load_scenarios.py --report "$env:TEMP\if_arena_0031_local_tcp_smoke.md"` - pass.
- `python tests\security\tcp_protocol_negative.py` - pass.
- `build\battle_cli_client.exe --fake-connect --create --match-id local-match --script tests\integration\server\cli_scenario_b.script` - pass.
- `git diff --check` - pass.
- `python scripts\ci\scan_secrets.py` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- `$env:Path = "C:\Program Files\Git\usr\bin;C:\Program Files\Git\bin;$env:Path"; & "C:\Program Files\Git\bin\bash.exe" -lc 'python3() { /c/Python314/python.exe "$@"; }; export -f python3; ./scripts/ci/validate_structure.sh'` - pass after removing local generated `build-qt-mingw/` artifacts.

## Test Impact Matrix

Changed behavior:

- TCP matches tick beyond tick `1200` while clients are idle.
- Playable backend matches no longer end at the core default tick `1200`.
- Movement submitted after tick `1200` is still accepted and applied authoritatively.
- Qt mouse movement no longer sends gameplay commands.
- Qt attack/dash controls and feedback are clarified.
- Idle no-op stop/input logging spam is removed.
- Arena readability indicators changed.

Tests added/updated:

- Happy path: `tests/integration/server/tcp_live_tick_smoke.py` covers an idle two-client TCP match ticking beyond `1200` and accepting movement afterward.
- Corner cases: duplicate idle stop intents are suppressed in Qt client code; manual checklist covers no idle spam and focus after join.
- Invalid input / hostile input: existing TCP/protocol/security checks retained; no parser or public protocol schema change.
- Authority / ownership: late movement test verifies authoritative server state changes only through submitted command; Qt still renders server snapshots only.
- Resource bounds / performance: TCP tick catch-up is bounded to four ticks per poll cycle; existing load/security checks will be rerun.
- Regression: Qt transform tests and existing TCP/load/security/protocol checks will be rerun.
- Manual UI checks: `tests/manual/qt_client_checklist.md` updated for mouse aim only, right click/Shift dash, attack/dash feedback, carrier/objective readability, and tick beyond `1200`.

Not tested and why:

- Direct Qt mouse/key event automation is not added in this task because the current Qt test target is a non-GUI transform test and the task scope does not allow broad test harness/CMake restructuring. The exact GUI behavior is covered by the manual checklist and by keeping mouse aim local in `ArenaView`/`NetworkClient`.
- Full two-window interactive Qt manual play was not executed in this non-interactive agent session. Qt compilation, Qt test execution, TCP live-tick regression, and the updated manual checklist cover the behavior until a human can click through both windows.

Known risks:

- TCP timer is implemented in the TCP runtime only because task 0031 targets Qt/raw TCP playability; WebSocket behavior is left unchanged.

Follow-up needed:

- Run automated checks and manual Qt checklist.

Ready for verification: yes

## Verification-Agent

Decision: approve-with-documented-manual-gap

Findings:

- No forbidden paths changed.
- New live TCP regression covers idle match ticking beyond `1200` and late movement application.
- Qt MinGW target builds and existing Qt transform tests pass.
- Existing protocol, backend, TCP, load, and security checks pass.
- `scripts/ci/validate_structure.sh` passed through Git Bash from `C:\Program Files\Git\bin` with Git `usr/bin` and a temporary `python3` function mapped to the installed Python.
- Full interactive two-window Qt manual play remains a human checklist item; not executable cleanly from this agent session.

Required re-checks:

- Re-run affected C++/Qt tests after review/fix edits.

## Review-Agent

Architecture-Agent:

Decision: approve

Findings:

- TCP timer remains in `battle_server_app` and does not move transport or Qt code into `battle_core`.
- Backend `maxTicks = 3600` is scenario configuration for playable backend matches, not a new gameplay mode.
- Clients still send intentions only; authoritative position/objective/combat remains server-owned.

Security-Agent:

Decision: approve

Findings:

- No public protocol schema or parser relaxation.
- Mouse aim is now local-only in Qt and accidental `Aim` intents are ignored by `NetworkClient`, reducing command spam surface.
- Existing TCP negative/security tests and secret scan pass.

Performance-Agent:

Decision: approve-with-minors

Findings:

- Active TCP tick catch-up is bounded to four ticks per poll cycle.
- Local TCP load smoke passes.
- Residual minor risk: current backend broadcasts snapshots on every tick because `snapshotRate` is not yet enforced in `MatchManager`; acceptable for this MVP bugfix at the local default `20Hz`, but future worker/snapshot coalescing should address this before higher-rate public configurations.

Qt-Agent:

Decision: approve-with-minors

Findings:

- Mouse movement no longer crosses the network boundary as gameplay input.
- Keyboard movement, attack, and dash routing stays in Qt signals/slots and sends intentions through `NetworkClient`.
- Focus is moved back to the arena after match join.
- Residual minor risk: GUI event behavior is covered by manual checklist rather than automated Qt event tests.

## Fix-Agent

Fixed findings:

- Adjusted Shift/Space direction fallback so keyboard actions use real mouse aim only after the user has moved the mouse; otherwise they fall back to movement direction, then local forward.

Changed files:

- `src/battle_qt_client/ui/MainWindow.hpp`
- `src/battle_qt_client/ui/MainWindow.cpp`

Checks:

- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake --build build-qt-mingw --parallel` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; ctest --test-dir build-qt-mingw --output-on-failure` - pass.
- `ctest --test-dir build --output-on-failure` - pass.
- `git diff --check` - pass.

Ready for re-verification: yes

## Verification-Agent After Fixes

Decision: commit-ready

Findings:

- Affected Qt build/test re-ran after fallback fix and passed.
- Default C++ ctest re-ran after fallback fix and passed.
- Secret scan, agent harness validation, playable task sequence validation, and diff whitespace check passed.
- Scope remains limited to task-allowed files.

## Commit-Agent

Decision: committed

- Commit: `playable: stabilize controls and arena readability` on branch `agent/0031-fix-playable-controls-and-arena-readability`.
- Scope: one task commit; no release tag created.
- Staged files matched task-allowed paths.

## Coordinator Closeout

Final status: committed.

Known risks:

- Full two-window Qt manual checklist still needs human execution.
- Snapshot coalescing / `snapshotRate` enforcement remains a future performance hardening item.

Follow-up tasks:

- Human-run `tests/manual/qt_client_checklist.md` against `build-qt-mingw\battle_qt_client.exe`.
- Keep release tagging blocked until explicitly authorized.
