# Rule Update Process

## When to add a rule

Add or update a rule when:

- a mistake is security-relevant;
- a mistake is expensive to fix;
- multiple agents repeat the same issue;
- a new subsystem introduces new constraints;
- CI/review found a gap in instructions.

## Where to add it

- Universal short rule: root `AGENTS.md`.
- Directory-specific behavior: nearest nested `AGENTS.md`.
- Reusable procedure: `.agents/skills/<skill>/SKILL.md`.
- Canonical policy: `docs/agent-rules/**`.
- One-off instruction: task packet only.

## Required update note

Harness updates should state:

- problem observed;
- chosen file changed;
- why this scope is appropriate;
- whether older docs were removed or linked.
