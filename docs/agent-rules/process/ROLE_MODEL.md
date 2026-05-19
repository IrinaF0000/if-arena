# Agent Role Model

## Coordinator Agent

Plans work, updates task packets, defines dependencies, and decides what can run in parallel. Does not perform large implementation work unless explicitly assigned.

## Implementation Agent

Implements one task packet in one branch/worktree. Keeps diffs small and scoped.

## Verification Agent

Runs checks, expands tests, validates build and behavior. Does not redesign features.

## Review Agent

Reviews diffs for correctness, maintainability, and missed tests. Reports findings with severity.

## Security Agent

Reviews untrusted input, auth, session ownership, secrets, logging, deployment exposure, and resource exhaustion.

## Performance Agent

Reviews tick loop, queues, metrics, load tests, latency, memory growth, and slow-client handling.

## Agent Manager

Maintains the harness: AGENTS.md hierarchy, skills, task packet quality, token economy, and recurring rule updates.
