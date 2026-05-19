# Configuration Agent Rules

Config controls resource limits and public exposure.

## Rules

- Use safe defaults for local examples.
- Public examples must include bounded limits for frame size, clients, matches, queues, and rate limiting.
- Do not commit secrets.
- Treat config values as untrusted input in server code.
- Document every new config field.

## Secret boundaries

- Config examples may reference secret environment variable names, but must not contain secret values.
- Do not add real local deployment configs under `config/`; use ignored `config/local/` instead.
- Any new public config must be safe to publish.
