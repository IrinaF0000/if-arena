# Task 0047: Repo hygiene and CI fixes

## Goal

Clear the post-0046 repository hygiene and CI blockers before gameplay/config work continues.

## Scope

Allowed files:

- `.gitattributes`
- `.gitignore`
- `scripts/**/*.sh` for line-ending normalization only
- `tests/frontend/websocket_local_smoke.py`
- `src/battle_server_app/main.cpp`
- `docs/agent-runs/0047-repo-hygiene-and-ci-fixes.md`
- `docs/agent-tasks/0047-repo-hygiene-and-ci-fixes.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**` logic changes
- `deploy/**`
- `docs/ci/**`
- `battle_core` gameplay rules
- scenario configs
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Add repository line-ending policy with `.gitattributes`.
- Normalize shell scripts to LF line endings without changing script logic.
- Ensure generated binaries, build outputs, Python caches, frontend dependencies, and frontend build output are ignored.
- Make the WebSocket local smoke test resolve the server executable path on Windows and non-Windows platforms.
- Make the warning-clean C++ build pass without globally suppressing warnings.
- Do not introduce config ownership, scenario runner, map, hazard, gameplay, UI, workflow, deploy, or CI script changes in this task.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `git status --short`
- `git status --ignored --short`
- `bash scripts/ci/validate_structure.sh`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-werror -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"`
- `cmake --build build-werror --parallel`
- `python tests/frontend/websocket_local_smoke.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, for executable-path smoke and config/startup safety
- Performance-Agent: no
- Frontend-Agent: no
- Qt-Agent: no
- CI/CD-Agent: no, because protected CI/CD files are not in scope
- Verification-Agent: yes

## Acceptance criteria

- No generated artefacts are staged.
- `.gitattributes` and `.gitignore` cover the post-0046 hygiene requirements.
- WebSocket smoke test uses a platform-aware server executable path.
- Warning-clean build passes or any blocker is recorded with exact compiler output.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to remove the hygiene and smoke/warning fixes.
