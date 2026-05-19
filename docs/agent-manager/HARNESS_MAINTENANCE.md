# Harness Maintenance

## Files maintained by the Agent Manager

- `AGENTS.md`
- nested `AGENTS.md` files
- `.agents/skills/**/SKILL.md`
- `docs/agent-rules/**`
- `docs/agent-tasks/**`
- `docs/agent-manager/**`
- `scripts/agent/**`

## Maintenance principles

- Prefer specific local instructions over broad root instructions.
- Prefer reusable skills over long task prompts.
- Keep security rules canonical in `docs/agent-rules/security/`.
- Keep quality rules canonical in `docs/agent-rules/quality/`.
- Do not duplicate full rule blocks across many files.
- Review harness changes like code changes.
