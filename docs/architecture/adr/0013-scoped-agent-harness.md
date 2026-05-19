# ADR 0013: Scoped Agent Harness

## Status

Accepted

## Context

The project uses coding agents across C++, Qt, TypeScript, TCP, WebSocket, Telegram Mini App, security, CI/CD, and high-load backend work. One large universal prompt is hard to maintain and expensive to load repeatedly.

## Decision

Use a scoped agent harness:

- root `AGENTS.md` as a short routing file;
- nested `AGENTS.md` files for subsystem-specific rules;
- `.agents/skills/**/SKILL.md` for reusable focused procedures;
- `docs/agent-rules/**` for canonical process, quality, security, and scalability policies;
- `docs/agent-manager/**` for maintaining the harness;
- `scripts/agent/**` for advisory validation and run summaries.

## Consequences

Agents load less irrelevant context, task prompts become shorter, protected areas are clearer, and recurring mistakes can be addressed with local rule updates instead of root prompt growth.

The Agent Manager role is responsible for preventing duplication and keeping the harness maintainable.
