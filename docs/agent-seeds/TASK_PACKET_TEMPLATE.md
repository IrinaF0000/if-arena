# Task Packet Template

Use this template for one scoped task. Keep packets short; reference canonical rule files instead of copying them.

## Task title
`<short imperative title>`

## Phase
`<phase from IMPLEMENTATION_PLAN.md or PLAYABLE_GAME_AGENT_PLAN.md>`

## Goal
`<one clear outcome>`

## Background
`<brief context and canonical plan/rule links>`

## Allowed files
- `<exact paths or directories>`

## Forbidden files
- `<paths not allowed>`
- Build outputs, generated dependency directories, and real secret files.

## Required quality gates
- Gate A
- Gate `<B-L as applicable>`

## Sequential agent pipeline
Follow `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`. Implementation-Agent must not commit.

## Implementation steps
1. `<small step>`
2. `<small step>`
3. `<small step>`

## Required tests
- Complete the Test Impact Matrix from `docs/agent-rules/quality/TESTING.md`.
- Add/update focused tests for every changed behavior.
- Cover happy path, corner cases, invalid/hostile input, authority violations, and resource bounds when touched.
- Record manual UI checks when automation is not practical.

## Required docs updates
- `<doc path or "none">`

## Acceptance criteria
- `<criterion 1>`
- `<criterion 2>`
- Required checks, reviews, and Test Impact Matrix are recorded in `docs/agent-runs/`.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes/no
- Security-Agent: yes/no
- Performance-Agent: yes/no
- Frontend-Agent: yes/no
- Qt-Agent: yes/no
- CI/CD-Agent: yes/no
- Verification-Agent: yes

## Rollback note
`<how to revert or isolate the change>`
