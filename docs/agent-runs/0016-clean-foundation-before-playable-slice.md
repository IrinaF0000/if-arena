# Task 0016: Clean Foundation Before Playable Slice

State: committed

## Coordinator Preflight

- Branch: `agent/0016-clean-foundation-before-playable-slice`.
- Task packet: `docs/agent-tasks/0016-clean-foundation-before-playable-slice.md`.
- Working tree before implementation: clean except pre-existing untracked `overlay.zip`, which is not part of this task and must not be staged.
- Required orchestration: `Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent after fixes -> Commit-Agent -> Coordinator closeout`.
- Required reviews: Coordinator, Architecture-Agent, Verification-Agent; Frontend-Agent if package files change.
- Required gates: Gate A, Gate H, Gate K, Gate L where applicable.
- Allowed files: `CMakeLists.txt`, placeholder source renames, run server scripts, frontend package files, frontend README, root README, `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`, and `docs/agent-runs/`.
- Forbidden files: `external/battle_simulation_snapshot/**`, `.github/workflows/**`, `deploy/**`, `docs/ci/**`.
- Preflight blocker found: `docs/project/PLAYABLE_GAME_AGENT_PLAN.md` is referenced by the orchestrator, task packets, and validator, but is missing from the repository. Because it is an allowed file for task 0016 and `validate_playable_task_sequence.py` requires it, Implementation-Agent should create the missing canonical plan with scenarios A-E and task ids 0016-0028.

## Implementation-Agent Prompt

Implement task 0016 only. Do not commit. Follow `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md` and `docs/agent-rules/quality/TESTING.md`.

Scope:

- Rename real C++ source files away from `placeholder.cpp` and update `CMakeLists.txt`.
- Fix `scripts/run_local_server.sh` and `scripts/run_public_server.sh` so they build/run `battle_server_app` or honestly document current non-playable behavior.
- Ensure generated frontend dependency/build directories are not tracked, and add/update frontend lockfile policy without adding dependencies unnecessarily.
- Create missing `docs/project/PLAYABLE_GAME_AGENT_PLAN.md` because the orchestrator and playable validator require it.
- Update `README.md` and frontend README with accurate playable status and next milestone.
- Update this run note with changed files, checks, Test Impact Matrix, risks, and readiness.

Constraints:

- Do not touch forbidden paths.
- Do not add gameplay behavior.
- Do not stage `overlay.zip`, build directories, `node_modules/`, or `dist/`.
- Keep changes mechanical and documentation-focused.

## Implementation-Agent Output

State transition: `ready -> implementing`.

Changed files:

- `CMakeLists.txt`
- `README.md`
- `frontend/telegram_mini_app/README.md`
- `scripts/run_local_server.sh`
- `scripts/run_public_server.sh`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- C++ source renames:
  - `src/battle_core/placeholder.cpp` -> `src/battle_core/BattleEngine.cpp`
  - `src/battle_protocol/placeholder.cpp` -> `src/battle_protocol/Protocol.cpp`
  - `src/battle_backend/placeholder.cpp` -> `src/battle_backend/BackendFoundation.cpp`
  - `src/battle_transport_tcp/placeholder.cpp` -> `src/battle_transport_tcp/TcpFrameCodec.cpp`
  - `src/battle_transport_ws/placeholder.cpp` -> `src/battle_transport_ws/WebSocketSession.cpp`
  - `src/battle_qt_client/placeholder.cpp` -> `src/battle_qt_client/QtClientMain.cpp`
  - `src/battle_cli_client/placeholder.cpp` -> `src/battle_cli_client/CliClientMain.cpp`
  - `src/battle_server/placeholder.cpp` -> `src/battle_server/ServerMain.cpp`
  - `tools/load_client/placeholder.cpp` -> `tools/load_client/LoadClientMain.cpp`

Implementation notes:

- No gameplay behavior was added.
- Run scripts now build and execute `battle_server_app` and print that the current server is a foundation executable, not a playable match server yet.
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md` was created because the orchestrator prompt and playable validator require it and task 0016 explicitly allows it.
- Frontend README documents that `node_modules/` and `dist/` are generated local artifacts and that lockfiles should be committed only when a dependency-change task generates them.

Checks run:

- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed, with Git line-ending warnings only.
- `python scripts/ci/scan_secrets.py` - passed after removing generated `package-lock.json`.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed after refreshing the ignored build tree to Ninja.
- `cmake --build build --parallel` - passed after adding missing `<iterator>` include for MSVC/GNU portability.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- `npm.cmd install --offline --ignore-scripts --audit=false --fund=false --loglevel=error` - passed using local cache. Plain online `npm install` timed out in this sandbox.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed when run outside the sandbox because the sandboxed Vite/esbuild config load hits an access-denied error.
- `C:\Program Files\Git\bin\bash.exe -lc 'python3() { "/c/Users/user/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe" "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'` - passed.

Test Impact Matrix:

Changed behavior:

- No gameplay behavior changed.
- Build source paths changed from placeholder filenames to semantic filenames.
- Local/public run scripts now point to `battle_server_app` instead of a missing old server path.
- Playable task sequence validation now has the required plan file.
- README status now accurately says the game is not playable yet.

Tests added/updated:

- Happy path: existing C++ tests passed through the required CMake build path.
- Corner cases: not applicable; mechanical rename and docs/run-script cleanup only.
- Invalid input / hostile input: not applicable; no parser, auth, network, or gameplay behavior changed.
- Authority / ownership: not applicable; no client/server authority logic changed.
- Resource bounds / performance: not applicable; no runtime resource behavior changed.
- Regression: CMake configure/build plus 6 CTest tests passed.
- Manual UI checks: not applicable; no UI behavior changed.

Environment notes:

- `bash scripts/ci/validate_structure.sh` requires Git Bash plus an explicit `python3` mapping to the bundled Python in this environment; passed with that mapping.
- Plain online `npm install`: timed out; offline/cache install passed and typecheck/lint/build passed.

Known risks:

- `overlay.zip` remains untracked and intentionally untouched.
- The current server starts, but it is not playable until later tasks add match/gameplay flow.

Ready for verification: yes.

## Verification-Agent Output

State transition: `implementing -> verifying`.

Diff scope:

- Only task-allowed paths changed.
- Forbidden paths were not changed.
- Generated dependency/build directories are ignored and not staged.
- `overlay.zip` is untracked and not staged.
- No generated `package-lock.json` is committed because it triggered secret-scan false positives and task 0016 does not change dependencies.

Verification decision: approve-with-environment-note.

Commands/results:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 6/6 tests.
- `git diff --check` - passed, with line-ending warnings only.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `npm.cmd install --offline --ignore-scripts --audit=false --fund=false --loglevel=error` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed outside sandbox.
- `C:\Program Files\Git\bin\bash.exe -lc 'python3() { "/c/Users/user/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe" "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'` - passed.

## Required Review-Agent Outputs

### Architecture-Agent

Decision: approve.

Findings:

- [info] Mechanical source renames preserve module boundaries and do not introduce forbidden dependencies.
- [info] Run scripts now target the correct composition executable and honestly state that playability is not implemented yet.
- [info] The playable plan records scenarios without claiming they are already implemented.

Required re-checks:

- None beyond Verification-Agent checks already run.

### Frontend-Agent

Decision: approve.

Findings:

- [info] Frontend code and package manifest were not changed; README dependency policy is accurate.
- [info] `node_modules/`, `dist/`, and generated lockfile output are not staged.
- [info] TypeScript typecheck, lint, and production build passed.

Required re-checks:

- None.

## Fix-Agent Output

State transition: `reviewing -> fixing`.

Fixed findings:

- Added missing `<iterator>` include in `src/battle_transport_tcp/TcpFrameCodec.cpp` after MSVC reported `std::back_inserter` was undeclared.
- Refreshed the ignored `build/` tree with Ninja so the required `ctest --test-dir build` command works without Visual Studio multi-config `-C` arguments.
- Removed generated `frontend/telegram_mini_app/package-lock.json` after secret scan flagged package integrity hashes; no dependency change is part of task 0016.

Changed files:

- `src/battle_transport_tcp/TcpFrameCodec.cpp`

Ready for re-verification: yes.

## Verification-Agent After Fixes

State transition: `fixing -> re-verifying -> commit-ready`.

Re-checks:

- Required CMake configure/build/test commands passed.
- Secret scan passed after removing generated lockfile.
- Frontend typecheck/lint/build passed.
- Playable task sequence validation passed.

Commit readiness:

- Commit-Agent may proceed with one scoped commit after confirming `git status --short` contains only task 0016 files plus the pre-existing untracked `overlay.zip`.

## Commit-Agent Output

State transition: `commit-ready -> committed`.

Commit:

- Final commit hash is recorded in the Coordinator closeout response. The hash is not embedded here because amending this run note changes the commit hash.

Commit scope confirmed:

- One scoped task commit.
- No forbidden files.
- No generated dependency directories or build outputs.
- `overlay.zip` remained untracked and unstaged.

## Coordinator Closeout

Final status: committed.

Known risks:

- `bash scripts/ci/validate_structure.sh` requires Git Bash plus an explicit `python3` mapping to the bundled Python in this environment; passed with that mapping.
- Plain online `npm install` timed out; offline/cache install plus typecheck/lint/build passed.
- The server executable is still not playable; task 0017 is the next task and should add the canonical Objective Run arena map/config in `battle_core`.

Follow-up:

- Start task `0017-core-arena-map-and-config.md` in a new task branch after this branch is merged/reviewed as appropriate.
