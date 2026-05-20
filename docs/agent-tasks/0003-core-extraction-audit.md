# Task: Audit old project for core extraction

## Phase

Phase 1. Original project import and core extraction

## Goal

Create an extraction map from old simulation code to new `battle_core` modules.

## Allowed files

- `external/battle_simulation_snapshot/`
- `docs/project/CORE_EXTRACTION_CHECKLIST.md`
- `docs/project/ORIGINAL_PROJECT_IMPORT.md`
- `docs/agent-notes/`

## Forbidden files

- `src/` implementation changes unless explicitly approved

## Required quality gates

- Gate A
- Gate H

## Security impact

low

## Performance/scalability impact

none

## Architecture impact

high

## Token budget instructions

- Use `rg` and file tree.
- Do not read every old file fully.
- Do not paste large old files into notes.

## Implementation steps

1. Inspect old project file tree.
2. Identify simulation state, systems, actions, config, replay, UI/tools, app entry points.
3. Write concise extraction map.
4. Mark files as core / app / tools / ignore.
5. Note risks and unclear dependencies.

## Required tests/checks

- Follow `docs/agent-rules/quality/TESTING.md`.
- Record Test Impact Matrix when behavior changes.
- Documentation-only check.
- Extraction map reviewed by Architecture-Agent.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope remains narrow.
- Checks pass or blockers are documented.
- Run note is updated.
