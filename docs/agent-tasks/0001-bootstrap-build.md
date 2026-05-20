# Task: Bootstrap placeholder build

## Phase

Phase 0. Repository bootstrap

## Goal

Make sure C++ placeholders configure/build and frontend placeholder structure is present.

## Allowed files

- `CMakeLists.txt`
- `.github/workflows/ci.yml`
- `frontend/telegram_mini_app/`
- `docs/project/REPOSITORY_BOOTSTRAP.md`

## Forbidden files

- `external/battle_simulation_snapshot/`
- Any production gameplay implementation

## Required quality gates

- Gate A
- Gate E if frontend scripts are changed

## Security impact

none

## Performance/scalability impact

none

## Architecture impact

low

## Token budget instructions

- Read only root CMake, CI, frontend package files, and bootstrap docs.
- Do not inspect old project snapshot.

## Implementation steps

1. Configure CMake with default options.
2. Build placeholder C++ targets.
3. Check frontend package scripts are consistent.
4. Update bootstrap docs if command names changed.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- CMake configure succeeds.
- CMake build succeeds.
- Frontend placeholder `npm run typecheck` and `npm run lint` commands are defined.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Verification-Agent: yes
- Architecture-Agent: no
- Security-Agent: no

## Acceptance criteria

- Scope remains narrow.
- Checks pass or blockers are documented.
- Run note is updated.
