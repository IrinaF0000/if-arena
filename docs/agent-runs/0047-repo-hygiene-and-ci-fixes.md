# Task 0047: Repo hygiene and CI fixes

## State

committed

## Coordinator Preflight

- Branch: `agent/0047-playable-stabilization`.
- Baseline: `master` after local task 0046 closeout merge.
- Working tree at preflight had one untracked local file not owned by this task: `post_0046_playable_stabilization_agent_plan.md`. It will remain unstaged unless explicitly requested.
- Ignored generated directories are present locally: build directories, frontend `node_modules/`, frontend `dist/`, and local runtime config. They must remain unstaged.
- Task packet created: `docs/agent-tasks/0047-repo-hygiene-and-ci-fixes.md`.
- Allowed scope confirmed: `.gitattributes`, `.gitignore`, shell script LF normalization only, WebSocket smoke executable path, possible server app warning-clean fix, focused task/run docs.
- Forbidden scope confirmed: no workflows, no `scripts/ci/**` logic changes, no deploy files, no scenario configs, no gameplay rules, no generated frontend `dist/**`, no release tag, no push, no merge into `master` or `main` before final post-0046 acceptance.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Architecture/Security Review-Agent -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0047 from Priority 1 of `post_0046_playable_stabilization_agent_plan.md`. Add repository hygiene line-ending and ignore rules, normalize shell scripts to LF without changing script logic, make `tests/frontend/websocket_local_smoke.py` resolve `battle_server_app` with a platform-aware executable name, and fix warning-clean build issues only if discovered. Do not change source gameplay, scenario configs, CI workflow logic, deploy files, or generated outputs. Do not merge to `master` or `main`.

## Implementation-Agent

Completed.

Changed files:

- `.gitattributes`
- `scripts/ci/validate_structure.sh`
- `scripts/run_local_server.sh`
- `scripts/run_public_server.sh`
- `src/battle_server_app/main.cpp`
- `tests/frontend/websocket_local_smoke.py`
- `docs/agent-tasks/0047-repo-hygiene-and-ci-fixes.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0047-repo-hygiene-and-ci-fixes.md`

Implementation summary:

- Added repository line-ending attributes for shell, Python, C++, CMake, YAML, TypeScript, JavaScript, JSON, and Markdown files.
- Normalized shell scripts to LF line endings without changing script logic.
- Reused the existing TCP smoke platform pattern for `tests/frontend/websocket_local_smoke.py`.
- Added explicit constructors for TCP and WebSocket runtime state aggregates so `-Wmissing-field-initializers` does not fail the warning-clean build.
- Confirmed `.gitignore` already covers the required generated artifacts: build directories, frontend `node_modules/`, frontend `dist/`, `*.exe`, `*.zip`, `*.pyc`, and `__pycache__/`.

## Test Impact Matrix

Changed behavior:

- Repository text files now have explicit line-ending policy.
- Shell scripts touched by this task use LF endings.
- WebSocket local smoke now resolves `battle_server_app.exe` on Windows and `battle_server_app` elsewhere.
- Server runtime state construction is explicit but runtime behavior is unchanged.

Tests added/updated:

- Happy path: `python tests/frontend/websocket_local_smoke.py` passed on Windows using the platform-aware executable path.
- Corner cases: non-Windows server executable resolution is covered by the same expression pattern already used by TCP smoke tests; not run on Linux in this workspace.
- Invalid input / hostile input: no parser, auth, network payload validation, or hostile input behavior changed.
- Authority / ownership: no gameplay authority or client-owned state path changed.
- Resource bounds / performance: no runtime limits, queues, ticks, or backpressure behavior changed.
- Regression: CMake configure/build, full CTest suite, warning-clean build, structure, secret, architecture, agent, playable-sequence validators, and diff check.
- Manual UI checks: none; no UI behavior changed.

Not tested and why:

- Linux execution of `python tests/frontend/websocket_local_smoke.py` was not available in this Windows workspace. The path fix mirrors the already-passing TCP smoke helper pattern and the Windows smoke passed.

## Verification-Agent

Passed.

Checks run:

- `git status --short`
- `git status --ignored --short`
- `bash -lc 'export PATH="/usr/bin:$PATH"; function python3(){ python "$@"; }; export -f python3; bash scripts/ci/validate_structure.sh'`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-werror -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
- `cmake --build build-werror --parallel`
- `python tests/frontend/websocket_local_smoke.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

Notes:

- Plain `bash scripts/ci/validate_structure.sh` initially failed in this Windows shell because Bash found Windows `find` and no `python3` executable. The re-run used Git Bash `/usr/bin` first and a temporary `python3` shell function mapped to the available Python executable.
- Plain `cmake -S . -B build-werror ...` initially created a Visual Studio cache without a compiler. The generated `build-werror` directory was removed after path verification, then reconfigured with Ninja and MinGW.
- `git diff --check` reported only line-ending conversion warnings and no whitespace errors.

## Review-Agent

Architecture-Agent: approved. The C++ change only makes server app runtime-state construction explicit; no dependencies, boundaries, gameplay rules, or `battle_core` isolation changed.

Security-Agent: approved. WebSocket smoke executable-path resolution does not weaken auth, parser, network input, session identity, logging, or config safety. Secret scan passed.

CI/CD-Agent: not required for workflow review. `scripts/ci/validate_structure.sh` was touched only for LF normalization; script logic was unchanged.

Performance Review Agent: not required. No runtime loops, queue limits, worker behavior, or performance claims changed.

## Commit-Agent

Completed.

- Commit: current task commit, `build: fix repo hygiene blockers`
- Branch: `agent/0047-playable-stabilization`
- No merge to `master` or `main`.
- No push or release tag.

## Coordinator Closeout

Task 0047 is terminal on the stabilization branch. The untracked local plan file `post_0046_playable_stabilization_agent_plan.md` remains unstaged.
