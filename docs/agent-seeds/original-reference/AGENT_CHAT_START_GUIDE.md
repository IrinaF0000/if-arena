# Agent Chat Start Guide

This guide is for optional local agent-assisted development. The public workflow summary lives in `docs/agent-workflow.md`.

## Setup

Copy templates into local active files when needed:

```text
docs/agent-seeds/AGENT_CONTEXT_TEMPLATE.md  -> AGENT_CONTEXT.md
docs/agent-seeds/AGENT_RULES_TEMPLATE.md    -> AGENT_RULES.md
docs/agent-seeds/AGENT_WORKFLOW_TEMPLATE.md -> AGENT_WORKFLOW.md
docs/agent-seeds/AGENT_PROGRESS_TEMPLATE.md -> AGENT_PROGRESS.md
```

Fill them before coding. Keep active progress files out of public commits unless they have been cleaned up into stable docs.

## First prompt

```text
Read AGENT_CONTEXT.md, AGENT_RULES.md, AGENT_WORKFLOW.md, and AGENT_PROGRESS.md.
Act as coordinator and create the next task packet.
```

## Role prompts

```text
Act as implementation-agent. Execute only the current task packet. Edit only allowed files.
```

```text
Act as verification-agent. Run only the listed checks and report results. Do not edit files.
```

```text
Act as review-agent. Review only the scoped diff and files. Do not edit files.
```

```text
Act as fix-agent. Fix only review findings within the current scope.
```

## Coordinator checklist

- one task;
- allowed/forbidden files;
- non-goals;
- verification commands;
- progress update.
