# Task 0042: Coverage baseline and test policy audit

## Goal

Add an informational local C++ coverage baseline and clarify the project test policy taxonomy without introducing CI coverage gates.

## Scope

Allowed files:

- `CMakeLists.txt`
- `docs/quality/COVERAGE_BASELINE.md`
- `docs/quality/TEST_PLAN.md`
- `docs/quality/TEST_POLICY_AUDIT_0042.md`
- `docs/agent-rules/quality/TESTING.md`
- `docs/agent-runs/0042-coverage-baseline-and-test-policy-audit.md`
- `docs/agent-tasks/0042-coverage-baseline-and-test-policy-audit.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- source gameplay/protocol/frontend behavior changes
- coverage percentage gates
- release tags
- pushing to GitHub

## Required behavior

- Add a local coverage instrumentation option for GNU/Clang builds.
- Add a `gcovr` coverage target when `gcovr` is available.
- Document how to generate local coverage reports and that coverage is informational only.
- Clarify unit, integration, smoke, browser E2E, and manual UI test categories.
- Audit old run-note policy at documentation level without rewriting historical run notes.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-coverage-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_ENABLE_COVERAGE=ON`
- `cmake --build build-coverage-mingw --parallel`
- `ctest --test-dir build-coverage-mingw --output-on-failure`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Normal build/test remains unaffected.
- Coverage instrumentation build/test passes locally with GNU/Clang.
- `gcovr` report generation is documented and available when `gcovr` exists on `PATH`.
- No CI workflow, deploy, or source behavior changes are made.

## Rollback note

Revert this task commit to remove coverage instrumentation and policy-audit docs.
