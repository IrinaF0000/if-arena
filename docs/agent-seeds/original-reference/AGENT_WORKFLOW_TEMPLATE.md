# Agent Workflow

## Active files

- `AGENT_CONTEXT.md`: stable context.
- `AGENT_RULES.md`: rules.
- `AGENT_PROGRESS.md`: current task.
- `docs/development-plan.md`: milestones.

## Roles

- **Coordinator**: selects one task, writes packets, updates progress.
- **Implementation-Agent**: edits only files allowed by the task.
- **Verification-Agent**: runs checks only; must not edit files.
- **Review-Agent**: reviews scoped diff/files only; must not edit files.
- **Fix-Agent**: edits only files needed for review findings.

## Cycle

1. Coordinator writes task packet.
2. Implementation-Agent implements.
3. Verification-Agent runs checks.
4. Review-Agent reports findings.
5. Fix-Agent fixes findings if needed.
6. Repeat checks/review if needed.
7. Coordinator updates progress and commits.

## Escalation

Ask the user only if requirements conflict, scope must expand, destructive action is needed, credentials/network are required, tools are missing, or repeated review cycles fail.

## Prompts

```text
Act as coordinator. Read root agent files and create the next task packet.
```

```text
Act as implementation-agent. Execute the current task packet only. Edit only allowed files.
```

```text
Act as verification-agent. Run listed checks only. Do not edit files.
```

```text
Act as review-agent. Review only scoped diff and files. Do not edit files.
```

```text
Act as fix-agent. Fix only review findings within current scope.
```
