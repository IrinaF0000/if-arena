# Task 0042: Coverage baseline and test policy audit

## State

completed

## Coordinator Preflight

- Branch: `agent/0042-coverage-baseline-test-policy-audit`.
- Baseline: `master` after local merge of task 0041.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0042-coverage-baseline-and-test-policy-audit.md`.
- Allowed implementation scope confirmed: CMake coverage option, quality/testing docs, task docs.
- Forbidden scope confirmed: no CI workflow changes, no `scripts/ci/**`, no deploy files, no source behavior changes, no coverage thresholds, no release tag, no push.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0042. Add an informational local C++ coverage baseline using GNU/Clang coverage instrumentation and optional `gcovr` report generation when available. Update testing docs to distinguish unit, integration, smoke, browser E2E, and manual UI checks. Do not modify CI workflows, scripts/ci, deploy files, or source behavior.

## Implementation-Agent

Completed.

Changed files:

- `CMakeLists.txt`
- `docs/quality/COVERAGE_BASELINE.md`
- `docs/quality/TEST_PLAN.md`
- `docs/quality/TEST_POLICY_AUDIT_0042.md`
- `docs/agent-rules/quality/TESTING.md`
- `docs/agent-tasks/0042-coverage-baseline-and-test-policy-audit.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0042-coverage-baseline-and-test-policy-audit.md`

Implementation summary:

- Added `BATTLE_ENABLE_COVERAGE` for GNU/Clang coverage instrumentation.
- Added an optional `coverage` target when `gcovr` is present on `PATH`.
- Documented local coverage commands, including the Windows MinGW/Ninja command used in this workspace.
- Documented that coverage is informational only and does not create a percentage gate.
- Clarified unit, integration, smoke, browser E2E, and manual UI test categories.
- Added a documentation-level audit for older run notes that predate the modern Test Impact Matrix.

## Test Impact Matrix

Changed behavior:

- CMake can optionally create coverage-instrumented GNU/Clang builds when `BATTLE_ENABLE_COVERAGE=ON`.
- Test policy docs now distinguish unit, integration, smoke, browser E2E, and manual UI checks.
- No gameplay, protocol, server, client, deployment, or CI workflow behavior changed.

Tests added/updated:

- Happy path: normal CMake configure/build/test and coverage CMake configure/build/test.
- Corner cases: coverage configure uses explicit Ninja/MinGW command on this Windows workspace to avoid the unsupported default Visual Studio generator path.
- Invalid input / hostile input: no runtime input handling changed; secret scan covers accidental sensitive data.
- Authority / ownership: no authoritative game-state ownership paths changed.
- Resource bounds / performance: coverage flags are opt-in and isolated to the coverage build directory.
- Regression: architecture, structure, agent harness, playable sequence, and diff checks.
- Manual UI checks: documented as human-only checks; not run because this task made no UI behavior changes.

Not tested and why:

- `gcovr` HTML/XML report generation was not run because `gcovr` is not installed in this workspace. Coverage instrumentation was verified by build/test plus raw gcov output files.
- Frontend typecheck/lint/build and browser E2E were not run because the task did not change frontend source or behavior.

Ready for verification: yes.

## Verification-Agent

Passed.

Checks run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-coverage-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_ENABLE_COVERAGE=ON`
- `cmake --build build-coverage-mingw --parallel`
- `ctest --test-dir build-coverage-mingw --output-on-failure`
- Raw coverage files after coverage CTest: 13 `.gcda`, 16 `.gcno`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`
- `bash scripts/ci/validate_structure.sh` with the local Windows `python3` shim

## Review-Agent

Architecture-Agent: passed. Coverage option is opt-in, limited to CMake build flags, and does not cross service boundaries.

Security Review Agent: not required. No network, auth, config parsing, Telegram, replay, or secret-bearing behavior changed; secret scan passed.

Performance Review Agent: not required. Runtime code is unchanged and coverage flags are isolated to an opt-in build.

## Fix-Agent

Not required.

## Commit-Agent

Pending local commit.

## Coordinator Closeout

Ready for local commit and merge. Do not stage the untracked local roadmap file `IF_Arena_Codex_Roadmap_After_Public_0033.md`.
