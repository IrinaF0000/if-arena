# scripts

Local development scripts.

## Manual run hygiene

Windows local play launchers start from a clean process state by calling:

```cmd
scripts\run\stop_if_arena.cmd
```

The stop script terminates tracked IF Arena server/client processes and only stops `node.exe` when it owns a known IF Arena port:

- `5555` - local TCP server for Qt/CLI testing.
- `8081` - local WebSocket server for the Telegram Mini App.
- `5173` - Vite dev server.

Use this when you intentionally want to keep Vite running while restarting server/client processes:

```cmd
scripts\run\stop_if_arena.cmd --keep-vite
```

Launcher logs and PID files are written under:

```text
build/run-logs/
build/run-state/
```
