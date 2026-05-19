# LLM Token Budget Rules

## Purpose

This project is large enough that careless agents can waste tokens and lose context. These rules are mandatory.

## General rules

- One task per agent run.
- Read only task-relevant files.
- Use file search and `rg` before opening files.
- Prefer small snippets over full-file reads.
- Do not paste full diffs unless needed.
- Do not paste generated files.
- Do not paste build logs longer than the relevant error section.
- Do not inspect dependency trees unless the task is dependency-related.
- Keep progress summaries short and factual.

## Forbidden to include in prompts

- `node_modules/`
- `build/`
- `.git/`
- compiled binaries;
- generated assets;
- full trace/replay files;
- full old project snapshot except in extraction/audit tasks;
- secrets or raw auth data.

## When context is too large

Create or update a local concise summary:

```text
docs/agent-notes/<task-name>-summary.md
```

Include:

- files inspected;
- relevant classes/functions;
- assumptions;
- unresolved questions.

Do not summarize unrelated modules.

## Suggested token budget by task type

```text
small doc update:       2k-5k context
small code task:        5k-15k context
architecture review:    10k-25k context
security review:        10k-25k context
core extraction task:   targeted chunks only, never whole snapshot
```

## Output discipline

Every agent final note should include:

- what changed;
- tests run;
- risks;
- next recommended task.

Avoid restating the entire project.
