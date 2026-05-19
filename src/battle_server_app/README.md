# battle_server_app

Composition root executable.

Owns:
- config loading;
- backend creation;
- TCP/WebSocket transport startup;
- metrics/log setup;
- graceful shutdown.

Keep business logic out of this module.
