# Task 0030: Release stabilization and Qt verification

## Task title
`Release stabilization and Qt verification`

## Phase
`Release candidate - v0.1.0-playable-mvp`

## Goal
Prepare IF Arena for a future `v0.1.0-playable-mvp` tag without creating the tag.

## Background
This is a release stabilization task after the playable MVP task sequence. Qt is installed locally and must be verified with the Qt MinGW kit.

## Scope
- Audit README and project docs for current status accuracy.
- Remove outdated skeleton-only wording where no longer true.
- Verify documented run commands.
- Verify TCP playable path, CLI path, load/security checks, and frontend build.
- Verify Qt build with installed Qt MinGW kit.
- Update docs with exact Windows Qt MinGW build instructions.
- Prepare concise release notes.
- Run full post-release checks.

## Non-goals
- Do not implement new gameplay features.
- Do not create a git tag.
- Do not change CI/deployment workflows.

## Qt verification command

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"
cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
cmake --build build-qt-mingw --parallel
ctest --test-dir build-qt-mingw --output-on-failure
```

## Required quality gates
- Gate A
- Gate C for client authority claims
- Gate D for Qt verification
- Gate F/G/H for load/security release notes
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

## Required reviews
- Coordinator: yes
- Verification-Agent: yes
- Review-Agent: yes, covering release docs, Qt command accuracy, frontend docs, and no-tag compliance.

## Rollback note
One scoped commit. Merge to `master` with a no-ff merge commit only after checks pass. Tagging requires explicit user authorization.
