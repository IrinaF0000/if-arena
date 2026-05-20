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

`battle_server_app` now loads and validates JSON config, initializes backend resource limits, logs safe startup/metrics summary, and exits with clear actionable errors when enabled listener transports are not implemented yet.

Useful local commands:

```bash
build/battle_server_app --config config/examples/server.local.json --check-config
build/battle_server_app --config config/examples/server.local.json
```

The first command validates config and backend initialization. The second currently fails before accepting clients if TCP/WebSocket listeners are enabled, which is expected until the later transport integration tasks wire real listeners.
