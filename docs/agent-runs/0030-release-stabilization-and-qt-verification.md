# Task 0030: Release Stabilization And Qt Verification

State: committed

## Coordinator Preflight

- Branch: `agent/0030-release-stabilization-and-qt-verification`
- Base: clean `master` at `71dcf4f`.
- Goal: prepare for a future `v0.1.0-playable-mvp` tag without creating the tag.
- Task packet: `docs/agent-tasks/0030-release-stabilization-and-qt-verification.md`
- Qt kit:
  - `CMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64`
  - MinGW bin: `C:\Qt\Tools\mingw1310_64\bin`
  - Ninja path: `C:\Qt\Tools\Ninja`
- Non-goals:
  - no new gameplay features;
  - no git tag;
  - no CI/deployment workflow changes.

## Implementation Assumptions

- This release stabilization task is documentation and verification focused.
- Build output directories are generated artifacts and must not be committed.
- Existing local TCP/CLI/WebSocket/frontend/load/security behavior remains unchanged.

## Implementation Note

Changed files:

- `.gitignore`
- `docs/agent-tasks/0030-release-stabilization-and-qt-verification.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0030-release-stabilization-and-qt-verification.md`
- `docs/operations/RELEASE_NOTES_v0.1.0-playable-mvp.md`
- `README.md`
- `src/battle_qt_client/README.md`
- `tests/manual/qt_client_checklist.md`
- `docs/architecture/TRANSPORT_ABSTRACTION.md`
- `docs/telegram/TELEGRAM_SECURITY.md`
- `docs/performance/PERFORMANCE_AND_LOAD_TESTING.md`
- `src/battle_qt_client/ui/ArenaView.cpp`

Summary:

- Added task packet and release stabilization run note for `0030-release-stabilization-and-qt-verification`.
- Added concise release candidate notes for future `v0.1.0-playable-mvp` tagging, explicitly stating that tagging is not part of this task.
- Updated README and Qt docs with exact Windows Qt MinGW commands using `C:\Qt\6.11.1\mingw_64`, `C:\Qt\Tools\mingw1310_64\bin`, and `C:\Qt\Tools\Ninja`.
- Removed outdated current-status wording around WebSocket skeleton-only behavior and public readiness.
- Updated performance wording so it does not overclaim async/production scale.
- Added `.gitignore` coverage for `build-*` CMake verification directories.
- Removed the only Qt MinGW build warning by making the local pen width literal a `qreal`.

Known risks:

- This is still a local playable MVP release candidate, not a production deployment.
- No git tag was created.
- Manual visual Qt checklist was updated but not interactively executed; the requested Qt configure/build/ctest path was verified.

Ready for verification: yes.

## Test Impact Matrix

Changed behavior:

- Release documentation and run commands now reflect the current playable MVP status.
- Qt MinGW release verification command is documented and verified.
- `build-*` generated CMake directories are ignored.
- Qt build warning is removed without changing gameplay behavior.

Tests added/updated:

- Happy path: verified default C++ build/test, Qt MinGW build/test, TCP vertical slice, real two-client CLI flow, WebSocket local smoke, frontend typecheck/lint/build, load/security smokes.
- Corner cases: existing CTest and protocol/frontend validation remain in the release check set.
- Invalid input / hostile input: `tcp_vertical_slice_smoke`, `websocket_local_smoke`, `tcp_protocol_negative`, and secret scan passed.
- Authority / ownership: existing protocol/frontend/Qt transform checks and TCP negative checks passed.
- Resource bounds / performance: load dry-run and local TCP load smoke passed.
- Regression: all release stabilization commands listed below passed.
- Manual UI checks: Qt manual checklist updated for the exact MinGW kit; not launched interactively in this task.

Not tested and why:

- No production deployment or tag creation was attempted by request.
- No new gameplay behavior was added.

## Verification

Commands run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass, 7/7 tests.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; cmake --build build-qt-mingw --parallel` - pass.
- `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"; ctest --test-dir build-qt-mingw --output-on-failure` - pass, 7/7 tests.
- `python tests\integration\server\tcp_vertical_slice_smoke.py` - pass.
- Real CLI two-client release smoke via `build\battle_server_app.exe` plus two `build\battle_cli_client.exe` processes - pass.
- `node tests\frontend\telegram_protocol_validation.mjs` - pass.
- `npm.cmd run typecheck` - pass.
- `npm.cmd run lint` - pass.
- `npm.cmd run build` - pass with escalation after sandbox blocked Vite/esbuild config access.
- `python tests\frontend\websocket_local_smoke.py` - pass.
- `python tests\load\load_client_dry_run.py` - pass.
- `python tests\load\local_tcp_load_scenarios.py --report build\local-tcp-smoke.md` - pass.
- `python tests\security\tcp_protocol_negative.py` - pass.
- `git diff --check` - pass.
- `python scripts\ci\scan_secrets.py` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- `scripts/ci/validate_structure.sh` through Git Bash with bundled Python shim - pass after removing generated `build-qt-mingw/`.

## Reviews

Review-Agent:

Decision: approve

Findings:

- Release notes are concise and do not claim production readiness.
- README and Qt docs contain the exact Windows Qt MinGW command requested by the user.
- No tag was created.
- Existing skeleton-only current-status wording was removed from active WebSocket/Telegram docs.
- The only code change is a warning cleanup in Qt rendering and does not alter gameplay behavior.

Required re-checks:

- None.

## Commit

Scoped task commit: `4ba0d33 release: stabilize playable mvp candidate`.
