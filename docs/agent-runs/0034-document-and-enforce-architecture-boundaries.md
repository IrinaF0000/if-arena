# Task 0034: Document and enforce architecture boundaries

## State

committed

## Coordinator Preflight

- Branch: `agent/0034-document-and-enforce-architecture-boundaries`.
- Baseline: `master` after public 0033.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0034-document-and-enforce-architecture-boundaries.md`.
- Allowed scope confirmed in the task packet: architecture boundary docs, architecture quality/review docs, root/docs/core agent instructions, `src/battle_core` README, `scripts/ci` architecture validator and structure hook, validator tests, this run note.
- Forbidden scope confirmed: no `.github/workflows/**`, `deploy/**`, production gameplay/protocol/transport/client changes, release tags, or task 0035 work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0034. Document subsystem and `battle_core` boundaries, add a lightweight architecture validator, wire it into structure validation, add focused validator tests, and update agent/review rules so future tasks require boundary enforcement. Record tolerated MVP exceptions explicitly. Do not refactor gameplay, start 0035, change workflows, deploy files, or stage the untracked roadmap.

## Implementation-Agent

Assumptions:

- `ArenaConfig.hpp` currently owning default Objective Run arena helpers and player-view transforms is a tolerated MVP exception, not a pattern to grow.
- Architecture validation should be lightweight and deterministic, with narrow checks that current code can pass.
- Production SVG safety should ignore the read-only `external/` snapshot.

Changed files:

- `AGENTS.md`
- `docs/AGENTS.md`
- `docs/agent-rules/quality/ARCHITECTURE.md`
- `docs/agent-runs/0034-document-and-enforce-architecture-boundaries.md`
- `docs/agent-tasks/0034-document-and-enforce-architecture-boundaries.md`
- `docs/agent-tasks/README.md`
- `docs/architecture/BATTLE_CORE_BOUNDARIES.md`
- `docs/architecture/SUBSYSTEM_BOUNDARIES.md`
- `docs/review/QUALITY_GATES.md`
- `scripts/ci/validate_architecture_boundaries.py`
- `scripts/ci/validate_structure.sh`
- `src/battle_core/AGENTS.md`
- `src/battle_core/README.md`
- `tests/ci/validate_architecture_boundaries_tests.py`

Implementation notes:

- Added focused subsystem and `battle_core` boundary docs, including narrow tolerated MVP exceptions for `ArenaConfig.hpp` default scenario helpers and player-view transforms in core.
- Added Gate M for architecture-boundary enforcement and linked future task instructions to the new boundary docs and validator.
- Added `validate_architecture_boundaries.py` to check forbidden production `battle_core` includes, selected forbidden CMake target dependencies, client/backend include boundaries, transport/UI include boundaries, and production SVG safety.
- Wired the architecture validator into `scripts/ci/validate_structure.sh`.
- Adjusted the existing structure validator large-file prune to skip `build-*` directories, matching the repo's documented `build-qt-mingw` Qt build directory and preventing local build outputs from being reported as repository files.
- Added focused Python tests for validator happy path, forbidden core includes, forbidden target dependencies, production SVG safety, and skipping the read-only `external/` snapshot.

Checks run:

- `python tests\ci\validate_architecture_boundaries_tests.py` - pass.
- `python scripts\ci\validate_architecture_boundaries.py` - pass.
- `git diff --check` - pass with Windows line-ending warnings only.
- `python scripts\ci\scan_secrets.py` - pass.
- `python scripts\agent\validate_agent_harness.py` - pass.
- `python scripts\agent\validate_playable_task_sequence.py` - pass.
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass.
- `bash scripts/ci/validate_structure.sh` - failed because `bash` is not in the PowerShell PATH.
- `& "C:\Program Files\Git\bin\bash.exe" scripts/ci/validate_structure.sh` - failed because Git Bash was launched without `usr/bin` tools and no `python3`.
- `$env:Path = "C:\Program Files\Git\usr\bin;C:\Program Files\Git\bin;$env:Path"; & "C:\Program Files\Git\bin\bash.exe" -lc 'python3() { /c/Python314/python.exe "$@"; }; export -f python3; ./scripts/ci/validate_structure.sh'` - failed once because local `build-qt-mingw/*.exe` files were detected as large files; fixed by pruning `build-*`.
- `$env:Path = "C:\Program Files\Git\usr\bin;C:\Program Files\Git\bin;$env:Path"; & "C:\Program Files\Git\bin\bash.exe" -lc 'python3() { /c/Python314/python.exe "$@"; }; export -f python3; ./scripts/ci/validate_structure.sh'` - pass after fix.

## Test Impact Matrix

Changed behavior:

- Future tasks have explicit architecture-boundary docs and a Gate M review rule.
- Repository structure validation now runs the architecture-boundary validator.
- Architecture validator rejects forbidden `battle_core` includes/dependencies, selected client/transport boundary violations, and unsafe production SVG content.
- Structure validation ignores local `build-*` output directories in its large-file scan.

Tests added/updated:

- Happy path: validator test covers a minimal valid repo with core/protocol/Qt-client targets and a safe production SVG.
- Corner cases: validator test covers external snapshot SVGs being skipped.
- Invalid input / hostile input: validator test covers forbidden production SVG script/external URL content.
- Authority / ownership: validator checks clients do not include backend internals, Qt client does not include `battle_core`, and transports do not include UI/client headers.
- Resource bounds / performance: not applicable; no runtime queues, workers, timers, or buffers changed.
- Regression: validator test covers forbidden `battle_core` include and forbidden `battle_core -> battle_protocol` CMake dependency.
- Manual UI checks: not applicable.

Not tested and why:

- Qt-specific build with `BATTLE_BUILD_QT_CLIENT=ON` was not rerun because this task does not change Qt code or CMake Qt target wiring. Existing default CMake/CTest and the architecture target scan passed.

Ready for verification: yes.

## Verification-Agent

Decision: pass after one Fix-Agent iteration for the structure validator build-directory prune.

Verified scope:

- No production gameplay, protocol, transport, frontend, deployment, workflow, or release-tag changes.
- Protected script changes are limited to `scripts/ci/validate_architecture_boundaries.py` and `scripts/ci/validate_structure.sh`.
- Untracked roadmap file `IF_Arena_Codex_Roadmap_After_Public_0033.md` remains outside task scope and unstaged.
- Generated `scripts/ci/__pycache__/` from validator tests was removed before commit readiness.

Required gates:

- Gate A: pass.
- Gate H: pass.
- Gate I: pass for limited `scripts/ci/**` validation plumbing.
- Gate K: pass.
- Gate M: pass.

## Review-Agent

Architecture-Agent:

- Decision: approve.
- Findings: boundary docs match `PROJECT_MAP.md`; validator checks the critical `battle_core` and subsystem dependency directions without forcing a large refactor; MVP exceptions are narrow and explicit.
- Required re-checks: keep `python scripts/ci/validate_architecture_boundaries.py` in structure validation.

Security-Agent:

- Decision: approve.
- Findings: no secrets, auth behavior, network parser behavior, or deployment exposure changed; SVG checks fail closed on script, `foreignObject`, and external URLs outside `external/`.
- Required re-checks: `python scripts/ci/scan_secrets.py` and structure validator.

CI/CD script review:

- Decision: approve.
- Findings: no `.github/workflows/**` changes; structure script now invokes the new validator and continues to run secret scan and agent harness validation. Pruning `build-*` avoids false positives from documented local build directories without allowing large tracked files.
- Required re-checks: `validate_structure.sh`.

## Fix-Agent

Fixed one verification finding:

- `validate_structure.sh` reported local `build-qt-mingw/*.exe` outputs as repository large files. Updated the existing large-file `find` prune to ignore `build-*` directories alongside `build`, `node_modules`, and `.git`.

Re-checks:

- `validate_structure.sh` through Git Bash with `python3` shim - pass.

## Commit-Agent

- Commit: `architecture: enforce subsystem boundaries`.
- Staged scope matched the task packet.
- Untracked roadmap file was not staged.

## Coordinator Closeout

- Final status: committed.
- Commit hash: recorded by Coordinator final response after commit creation.
- No push, tag, workflow change, deployment file change, production gameplay change, or task 0035 work was performed.
- Known risk: the roadmap file remains untracked in the workspace by design; keep it out of task commits unless the user explicitly asks to version it.
- Follow-up: task 0035 can start after this branch is accepted/merged per roadmap policy.
