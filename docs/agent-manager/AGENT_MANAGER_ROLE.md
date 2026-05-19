# Agent Manager Role

The Agent Manager maintains the repository's agent harness as a product.

## Goals

- Help agents produce safe, high-quality, reviewable code.
- Keep instructions scoped and short.
- Reduce repeated context loading.
- Prevent accidental changes to protected areas.
- Improve task packets and skills based on recurring failures.

## Non-goals

- Implementing large features directly.
- Replacing code review.
- Adding broad rules for one-off mistakes.

## Regular checks

- Are root instructions still short?
- Are directory AGENTS.md files focused?
- Are skills discoverable and not duplicated?
- Are task packets specific enough?
- Are CI/CD guardrails still enforced?
- Are security and token-economy rules being followed?
