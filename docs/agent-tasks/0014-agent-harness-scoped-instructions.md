# Task 0014: Maintain Scoped Agent Instructions and Skills

## Goal

Keep the repository agent harness small, scoped, secure, and token-efficient.

## Allowed files

- `AGENTS.md`
- nested `AGENTS.md` files
- `.agents/skills/**/SKILL.md`
- `docs/agent-rules/**`
- `docs/agent-manager/**`
- `scripts/agent/**`
- `docs/agent-runs/**`

## Forbidden files

- production source code unless explicitly requested;
- `.github/workflows/**` unless this task is combined with CI/CD guardrails approval;
- `deploy/**`;
- `external/battle_simulation_snapshot/**`.

## Requirements

- Keep root `AGENTS.md` short.
- Put detailed procedures into focused rule files or skills.
- Do not duplicate long rule blocks.
- Preserve core requirements: security, code quality, access boundaries, agent roles, scalability, token economy.
- Update `scripts/agent/validate_agent_harness.py` when adding required harness files.

## Checks

```bash
python3 scripts/agent/validate_agent_harness.py
bash scripts/ci/validate_structure.sh
```

## Review gates

- Agent Manager review required.
- Security review required if security rules are weakened or moved.
- CI/CD review required if workflow or CI script behavior changes.
