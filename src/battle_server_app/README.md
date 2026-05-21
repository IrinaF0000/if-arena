# battle_server_app

Composition root executable.

Owns:
- config loading;
- backend creation;
- TCP/WebSocket transport startup;
- metrics/log setup;
- graceful shutdown.

Keep business logic out of this module.

## Current behavior

`battle_server_app` now loads and validates JSON config, initializes backend resource limits, and starts the local raw TCP listener when `transports.tcp.enabled` is true. If TCP is disabled and WebSocket is enabled, it starts the local WebSocket listener for Telegram Mini App/browser development.

Useful local commands:

```bash
build/battle_server_app --config config/examples/server.local.json --check-config
build/battle_server_app --config config/examples/server.local.json --max-clients 2
```

The first command validates config and backend initialization. The second starts TCP and exits after two accepted clients, which is useful for local smoke tests.

The TCP slice maps each accepted connection to a backend session, validates every client envelope, closes malformed or oversized frames, enforces handshake and idle timeouts, and uses backend outbound queue limits for slow-client backpressure.

For WebSocket local development, use a config with `transports.tcp.enabled=false` and `transports.websocket.enabled=true`. Public deployment still requires WSS/TLS and production auth hardening.
