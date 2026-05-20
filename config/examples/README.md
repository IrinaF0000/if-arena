# examples

Example local/server/client configs.

## Server fields

- `mode`: `local` or `public`. Public mode enables stricter validation.
- `server.tickRate` / `server.snapshotRate`: deterministic backend timing targets.
- `server.maxConnections` / `server.maxMatches`: backend capacity limits.
- `transports.tcp` and `transports.websocket`: listener intent and frame/message limits. The current server app validates these but exits with an actionable error until listener integration lands.
- `security.demoAuthEnabled`: allowed for local demos only.
- `security.telegramAuthEnabled`: public Telegram auth switch. Secrets are referenced by environment variable name only.
- `security.maxInputCommandsPerSecond`, `maxPendingWriteBytesPerSession`, `maxPendingOutboundMessages`, `maxPendingCommandsPerSession`, `maxPendingCommandsPerMatch`, and `maxCommandsPerTick`: explicit resource bounds.
- `metrics.logEverySeconds`: safe metrics log cadence.
