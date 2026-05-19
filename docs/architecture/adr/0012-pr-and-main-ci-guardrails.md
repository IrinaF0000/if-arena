# ADR 0012: Separate PR CI and main CI with protected workflow changes

## Status

Accepted

## Context

The project will be developed with multiple agent branches and Codex worktrees. CI/CD workflows are security-sensitive because they can access repository metadata, run arbitrary code, build artifacts, and potentially use secrets for deployment.

## Decision

Use separate workflow responsibilities:

- PR CI validates pull requests and never deploys.
- Main CI validates the merged tree and may build release-style artifacts.
- Deployment remains manual or environment-protected until explicitly implemented.
- `.github/workflows/**` is a protected path for agents and may only be changed by dedicated CI/CD tasks.
- CI/CD rules are documented in `docs/ci/CI_CD_GUARDRAILS.md`.

## Consequences

Benefits:

- safer agent-driven development;
- clearer PR checks;
- less risk of accidental deployment;
- easier review of workflow changes;
- better separation between implementation and operations.

Trade-offs:

- workflow changes require extra review;
- some tasks may need a separate follow-up PR to update CI;
- initial setup is slightly more verbose.
