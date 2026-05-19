# LLM Token Economy Rules

## Required behavior

- Read the nearest `AGENTS.md` and the assigned task packet first.
- Use targeted file reads and search. Do not scan the whole repository without a reason.
- Do not paste full files in chat or progress notes.
- Summarize changed files and decisions briefly.
- Prefer local edits over file rewrites.
- Do not regenerate existing docs unless the task explicitly asks for it.
- Do not create alternative architecture proposals after architecture is approved, unless a blocking issue is found.
- Avoid long generic comments in code.

## Subagents

Use subagents only for bounded independent tasks such as security review, test gap analysis, or protocol risk analysis. Do not use subagents for trivial edits.

## Reports

A good report contains:

- changed files;
- checks run;
- important decisions;
- risks;
- next steps.

It does not contain full diffs or repeated documentation.
