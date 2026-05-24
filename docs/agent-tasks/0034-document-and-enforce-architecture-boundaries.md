# Task 0034: Document and enforce architecture boundaries

## Goal

Make subsystem ownership and `battle_core` isolation explicit and enforceable before adding more gameplay features.

## Scope

Allowed files:

- `docs/architecture/SUBSYSTEM_BOUNDARIES.md`
- `docs/architecture/BATTLE_CORE_BOUNDARIES.md`
- `docs/agent-rules/quality/ARCHITECTURE.md`
- `docs/review/QUALITY_GATES.md`
- `docs/agent-tasks/0034-document-and-enforce-architecture-boundaries.md`
- `docs/agent-tasks/README.md`
- `docs/agent-runs/0034-document-and-enforce-architecture-boundaries.md`
- `AGENTS.md`
- `docs/AGENTS.md`
- `src/battle_core/AGENTS.md`
- `src/battle_core/README.md`
- `scripts/ci/validate_architecture_boundaries.py`
- `scripts/ci/validate_structure.sh`
- `tests/ci/validate_architecture_boundaries_tests.py`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- production C++ or TypeScript implementation other than the validator script
- gameplay refactors
- release tags
- task 0035 or later

## Required behavior

- Document subsystem ownership and forbidden dependency directions.
- Document internal `battle_core` layering: minimal foundation, gameplay features, scenarios/map config, view/input transforms, and IO outside core.
- Add a lightweight architecture validator and wire it into `scripts/ci/validate_structure.sh`.
- Validator fails on forbidden `battle_core` includes and forbidden CMake target dependencies where feasible.
- Validator also checks client/backend and transport/client boundary includes where feasible.
- Validator checks production SVG assets for `script`, `foreignObject`, and external URLs.
- Current temporary MVP exceptions are explicit and narrow:
  - `ArenaConfig.hpp` may currently contain default Objective Run arena helpers.
  - player-view transform helpers may currently live in core until extracted.

## Required quality gates

- Gate A. Every implementation task
- Gate H. Documentation-only changes
- Gate I. CI/CD and workflow changes, limited to `scripts/ci/**` validation plumbing
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `python tests/ci/validate_architecture_boundaries_tests.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `git diff --check`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh`
- CMake configure/build/CTest unless documented as not affected

## Required reviews

- Architecture-Agent: yes
- Security-Agent: yes, because protected validation scripts and asset safety checks are touched
- CI/CD-Agent or Coordinator script review: yes, because `scripts/ci/**` changes
- Verification-Agent: yes

## Acceptance criteria

- Boundary docs exist and match `PROJECT_MAP.md`.
- Agent/review rules require architecture-boundary review for future tasks.
- Architecture validator is wired into structure validation.
- Validator and tests pass on the current repository.
- No production gameplay, protocol, transport, client, workflow, deployment, or release-tag changes are made.

## Rollback note

One scoped task commit. Revert the validator, docs, task packet, and `validate_structure.sh` hook if the boundary rules need to be replaced.
