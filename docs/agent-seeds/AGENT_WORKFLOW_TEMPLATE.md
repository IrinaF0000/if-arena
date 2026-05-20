# Agent Workflow

Use this as a compact starter. Canonical rules live in focused files; do not duplicate them in prompts or task packets.

## Read first

1. Assigned task packet in `docs/agent-tasks/`.
2. `PROJECT_MAP.md`.
3. Nearest `AGENTS.md` for changed paths.
4. Required gates in `docs/review/QUALITY_GATES.md`.
5. Testing policy in `docs/agent-rules/quality/TESTING.md`.
6. For role-based work, `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.

## Roles

- Coordinator: selects one task, defines scope/gates/reviews, owns state and closeout.
- Implementation-Agent: makes the smallest scoped change, adds tests, does not commit.
- Verification-Agent: checks diff, tests, gates, Test Impact Matrix, and forbidden paths.
- Review-Agent: gives an explicit approve/request-changes/block decision for its specialty.
- Fix-Agent: fixes only listed findings.
- Commit-Agent: commits only `commit-ready` scoped changes.

Specialist mapping is defined in `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.

## Standard order

Follow `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.

## Token-saving rules

- Use `rg` before opening broad code areas.
- Open only files needed for the task and gates.
- Do not read `node_modules/`, `build/`, generated assets, or the full old snapshot unless required.
- Prefer links to canonical docs over copying rules.

## Stop conditions

Stop for Coordinator decision if the task needs forbidden files, a new dependency, a public boundary change beyond scope, a CI/CD-sensitive path, weakened checks, or hidden security/performance tradeoffs.
