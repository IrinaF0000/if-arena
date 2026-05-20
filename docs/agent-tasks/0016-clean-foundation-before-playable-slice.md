# Task 0016: Clean foundation before playable slice

## Task title
`Clean foundation before playable slice`

## Phase
`Phase 0/12 - foundation cleanup`

## Goal
Prepare the repository for playable-game work by removing placeholder naming, fixing local run scripts, documenting strict build expectations, and preventing generated frontend dependencies from being treated as source.

## Background
See `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.

## Allowed files
- `CMakeLists.txt`
- `src/**/placeholder.cpp and matching headers only when renaming placeholders`
- `scripts/run_local_server.sh`
- `scripts/run_public_server.sh`
- `frontend/telegram_mini_app/package.json`
- `frontend/telegram_mini_app/package-lock.json if generated`
- `frontend/telegram_mini_app/README.md`
- `README.md`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `docs/agent-runs/`

## Forbidden files
- `external/battle_simulation_snapshot/**`
- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`

## Required quality gates
- Gate A
- Gate H
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity, where applicable

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Rename source files so real code no longer lives in `placeholder.cpp`.
2. Update CMake target source lists after renames.
3. Fix run scripts to call the actual `battle_server_app` target or document that the server is not playable yet.
4. Remove generated `node_modules/` and `dist/` from version control if present.
5. Add or update lockfile policy for the frontend.
6. Add a short README note explaining the current playable status and next milestone.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required docs updates
As needed.

## Acceptance criteria
- CMake configure/build/test passes.
- Run scripts no longer point to missing binaries.
- No generated frontend dependency directory is staged.
- No behavior is added beyond cleanup.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes if package files change
- Verification-Agent: yes

## Rollback note
One scoped commit.
