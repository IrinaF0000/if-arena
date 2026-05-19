# Server App Agent Rules

The server app wires config, backend, transports, logging, metrics, and shutdown together.

## Rules

- Keep orchestration thin.
- Do not place gameplay rules in `main.cpp`.
- Validate config before starting network listeners.
- Public deployments must require explicit host/port/TLS/auth configuration.
- Graceful shutdown must stop accepting clients, close sessions, and flush safe metrics without dumping secrets.

Read also: `docs/agent-rules/security/DEPLOYMENT.md` and `docs/agent-rules/scalability/BACKEND_SCALABILITY.md`.
