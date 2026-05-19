# Task: Import original project snapshot

## Phase

Phase 1. Original project import and core extraction

## Goal

Copy the old local simulation project into the snapshot directory without modifying it.

## Allowed files

- `external/battle_simulation_snapshot/`
- `docs/project/ORIGINAL_PROJECT_IMPORT.md`
- `docs/project/CORE_EXTRACTION_CHECKLIST.md`

## Forbidden files

- `src/` production code except docs updates
- Old original repository outside this project

## Required quality gates

- Gate A
- Gate H

## Security impact

low

## Performance/scalability impact

none

## Architecture impact

medium

## Token budget instructions

- Do not paste the full snapshot into prompts.
- Use file lists and concise summaries only.

## Implementation steps

1. Copy the original project files into `external/battle_simulation_snapshot/`.
2. Remove build outputs if copied accidentally.
3. Add or update snapshot README with source commit/date if available.
4. Do not refactor imported files.

## Required tests/checks

- Confirm no build artifacts or binaries are committed.
- Confirm snapshot README exists.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Coordinator: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope is not broadened.
- Required checks pass or blockers are documented.
- Agent progress is updated with files changed, tests run, and remaining risks.
