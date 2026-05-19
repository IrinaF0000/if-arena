# Agent Workflow

## Active files

- `AGENT_CONTEXT.md`: stable project context.
- `AGENT_RULES.md`: mandatory rules.
- `AGENT_PROGRESS.md`: current task packet and progress.
- `docs/project/IMPLEMENTATION_PLAN.md`: phase plan.
- `docs/security/SECURITY_REQUIREMENTS.md`: security requirements.
- `docs/security/THREAT_MODEL.md`: threat model.
- `docs/review/QUALITY_GATES.md`: completion gates.
- `docs/review/CODE_REVIEW_CHECKLIST.md`: review checklist.
- `docs/ci/CI_CD_GUARDRAILS.md`: CI/CD workflow rules.
- `docs/agent-tasks/`: ready-to-use implementation tasks.

## Roles

### Coordinator

- Selects exactly one task.
- Writes or updates the task packet.
- Defines allowed files and forbidden files.
- Determines required quality gates.
- Determines required specialist reviews.
- Keeps token budget small by pointing agents to specific files.

### Architecture-Agent

Required when a task changes:

- module boundaries;
- public APIs;
- protocol;
- transport abstraction;
- ownership/lifetime model;
- threading model;
- CMake target dependencies;
- extraction from the old project.

Checks:

- dependency direction;
- separation of core/backend/transport/client;
- no rule leakage to clients;
- no transport leakage to core.

### Security-Agent

Required when a task touches:

- socket/WebSocket handling;
- protocol parsing;
- JSON parsing;
- Telegram auth;
- session identity;
- rate limiting;
- logging;
- config;
- deployment;
- frontend network code.

Checks:

- hostile input;
- size limits;
- auth validation;
- secret redaction;
- fail-closed behavior;
- negative tests.

### Performance-Agent

Required when a task touches:

- async I/O;
- queues;
- match workers;
- tick loop;
- broadcasting;
- load client;
- metrics;
- frontend render loop.

Checks:

- bounded resources;
- no thread-per-client;
- no blocking hot paths;
- latency/throughput measurement;
- memory growth.

### Frontend-Agent

Required for Telegram Mini App tasks.

Checks:

- strict TypeScript;
- WebSocket state machine;
- runtime validation;
- no secrets;
- responsive UI;
- mobile and desktop controls.

### Qt-Agent

Required for Qt UI tasks.

Checks:

- no blocking UI;
- signal/slot boundaries;
- separation of network and widgets;
- rendering clarity;
- input handling.


### CI/CD-Agent

Required when a task touches:

- `.github/workflows/**`;
- `.github/actions/**`;
- `.github/CODEOWNERS`;
- `scripts/ci/**`;
- `deploy/**`;
- `docs/ci/**`.

Checks:

- PR CI remains validation-only;
- main CI remains separate;
- workflow permissions are least-privilege;
- production secrets are not exposed to PRs;
- workflow changes are not mixed with unrelated feature work;
- required checks and rollback plan are documented.

### Verification-Agent

Runs tests, reviews diffs, and confirms acceptance criteria.

### Fix-Agent

Fixes issues found by review or tests. Must not expand scope.

## Standard workflow

1. Coordinator selects one task.
2. Coordinator writes task packet from `TASK_PACKET_TEMPLATE.md`.
3. Implementation-Agent inspects only relevant files.
4. Implementation-Agent writes short findings summary.
5. Implementation-Agent makes minimal changes.
6. Implementation-Agent runs focused checks.
7. Specialist agents review if required.
8. Verification-Agent runs required gates.
9. Fix-Agent resolves issues if needed.
10. Coordinator updates progress and moves to next task.

## Token-saving workflow

Before reading code:

```text
1. Read task packet.
2. Read project map.
3. Use file tree / rg to locate likely files.
4. Open only relevant files.
5. Summarize assumptions.
```

Before editing docs:

```text
1. Update only docs affected by the change.
2. Do not rewrite large docs for small code changes.
3. Add focused notes instead of duplicating content.
```

Before asking another agent:

```text
1. Provide concise context.
2. List exact files changed.
3. List exact questions.
4. Avoid dumping full diffs unless necessary.
```

## Escalation

Stop and request Coordinator decision if:

- the task requires changing a forbidden file;
- a module boundary must change;
- a security requirement conflicts with the requested implementation;
- a dependency addition is needed;
- a CI/CD-sensitive path must change;
- a large rewrite seems necessary;
- tests reveal old behavior differs from expected behavior.
