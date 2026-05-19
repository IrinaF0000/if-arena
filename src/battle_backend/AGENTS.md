# battle_backend Agent Rules

`battle_backend` owns server-authoritative gameplay orchestration above `battle_core` and below transports.

## Responsibilities

- Session-independent backend model.
- Match manager and match workers.
- Player ownership and server-side command validation.
- Rate limiting, backpressure policy, bounded queues.
- Metrics and observability counters.
- Transport-neutral command and event routing.

## Security and scalability

- Never trust a client-reported player id, team, position, HP, score, cooldown, or match state.
- Enforce max clients, max matches, max commands per second, max queue sizes.
- Slow clients must not cause unbounded memory growth.
- Each match should be owned by one worker or actor-like execution context.
- Shared mutable state must be explicit and protected.

Read also: `docs/agent-rules/scalability/BACKEND_SCALABILITY.md` and `.agents/skills/highload-backend/SKILL.md`.
