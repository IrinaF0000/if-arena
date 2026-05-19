# Agent Workflow

This file describes the optional AI-agent workflow used while developing this repository. It is included to make the process transparent, scoped, and reproducible.

## Project identity

- Project: `battle_simulation`
- Executable: `battle_sim`
- Namespace: `battle_sim`
- Scope: C++20 ECS-style turn-based battle simulation sandbox
- Current focus: modular gameplay-style architecture, deterministic scenarios, testable feature changes, and controlled AI-assisted development

## Stable rules

- Keep changes task-scoped and easy to review.
- Preserve command syntax and `UNIT_*` compatibility output unless a task explicitly changes it.
- Describe agents as support tools; humans own design, review, and validation.
- Require acceptance criteria and verification commands before implementation.
- Do not write C-style casts. Remove unnecessary casts; otherwise use explicit C++ casts.
- Avoid known C++/OOP antipatterns: raw owning pointers, hidden global state, god objects, inheritance for code reuse, needless downcasts, macro-driven logic, and exceptions as normal control flow.
- Treat `*_development_plan.md` files as local planning artifacts unless a task explicitly says otherwise; keep them ignored, untracked, and out of commits.

## Roles

- **Coordinator**: selects one task, writes the task packet, checks scope, and decides when a change is ready to commit.
- **Implementation agent**: edits only files allowed by the task packet.
- **Verification agent**: runs checks only and does not edit files.
- **Review agent**: reviews scoped diffs and files only.
- **Fix agent**: fixes only review findings inside the current scope.

## Task packet template

```text
Step:
Goal:
Allowed files:
Forbidden files:
Out of scope:
Required context:
Acceptance criteria:
Verification commands:
Review scope:
```

## Workflow cycle

1. Write a task packet.
2. Implement the task.
3. Run build, tests, scenarios, and `git diff --check`.
4. Review behavior and architecture boundaries.
5. Fix review findings.
6. Update documentation after verification.
7. Commit only after the task is complete and verified.

## Architecture rules for AI agents

- Keep `src/Core/` domain-neutral: ECS storage, resources, scheduler, registries, type-erased events, mutations, and feature-pack interfaces.
- Place battle components, resources, policies, selectors, effects, mutations, events, and systems under `src/Features/Battle/`.
- Place built-in unit recipes and unit-specific action rules under `src/Features/UnitsClassic/`.
- Use `src/App/` for CLI wiring, configuration loading, and scenario execution.
- Use `src/IO/LegacyCommands/` for text-command parsing and legacy output formatting.

## Dependency rules

```text
App -> IO/LegacyCommands
App -> Features/*
IO/LegacyCommands -> Features/Battle + Core event sink interface
Features/* -> Core
Core -> standard library only and Core-internal headers
```

Dependency changes must be explicit in the task packet and reviewed before merge.

## Verification and review

Baseline verification:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/battle_sim commands_example.txt
./build/battle_sim commands_validation.txt
git diff --check
```

Review should cover:

- task scope and changed files;
- behavior and test coverage;
- architecture boundaries;
- dependency direction;
- generated artifacts and local files;
- clarity of documentation updates.

Example case study:

- `docs/case-studies/add-json-trace-agent-run.md`

## Commit rules

Agents may create commits only after the assigned task is complete and verified.

Before committing, the agent must ensure that:

- the task scope is explicit;
- the diff is limited to the approved files or approved task area;
- build, tests, scenario validation, and `git diff --check` pass;
- generated files, build artifacts, caches, and local IDE files are not included;
- local development plan files such as `*_development_plan.md` are not staged or committed;
- the commit message clearly describes the actual change.

Agents must keep commits small and task-scoped.

Agents must not mix unrelated changes in one commit, such as:

- behavior changes and broad refactoring;
- documentation updates and functional changes;
- cleanup and new features;
- formatting-only changes and logic changes.

Agents must not push, merge, tag releases, rewrite history, change repository settings, or modify CI/security configuration without explicit human approval.

For larger, risky, or architecture-affecting changes, the agent prepares a diff and summary first. The human owner reviews the change and decides whether it should be committed.
