# Deployment Guide

## 1. Components

Public demo has three deployable components:

1. `battle_server_app`
   - C++ backend process.
   - Exposes raw TCP for Qt/CLI/load clients.
   - Exposes WebSocket for Telegram Mini App clients.

2. Telegram Mini App frontend
   - Static TypeScript/Vite build.
   - Served over HTTPS.

3. Telegram bot configuration
   - Bot button/menu opens Mini App URL.
   - Bot token must stay server-side or in secret storage.

## 2. Ports

Example local:

```text
TCP game port:       5555
WebSocket port:      8081
Mini App frontend:   5173
```

Example public:

```text
raw TCP:             public-host:5555
WebSocket/WSS:       wss://game.example.com/ws
Mini App HTTPS:      https://app.example.com/
```

## 3. Public security requirements

- Use WSS for Mini App.
- Use HTTPS for frontend.
- Keep bot token in environment variable or secret manager.
- Do not commit secrets.
- Use conservative public config.
- Expose only required ports.
- Apply firewall/security group rules.
- Disable demo auth unless explicitly intended.
- Rate-limit unauthenticated sessions.
- Enable log redaction.

## 4. TLS strategy

MVP options:

- Reverse proxy terminates TLS and forwards WebSocket to backend.
- Platform-managed HTTPS/WSS if available.
- Direct TLS in C++ backend as stretch goal.

## 5. Raw TCP hosting

Raw TCP requires a platform that supports public TCP ports. A regular HTTP-only platform is not enough for Qt raw TCP.

## 6. Telegram Mini App hosting

Telegram Mini App frontend must be served over HTTPS for public use.

The frontend must know the public WSS endpoint through build-time or runtime config. Do not store secrets in frontend config.

## 7. Environment variables

Example:

```text
BATTLE_CONFIG=/etc/battle/server.public.json
BATTLE_TELEGRAM_BOT_TOKEN=...
BATTLE_LOG_LEVEL=info
```

## 8. Docker

Server Docker image should contain only runtime artifacts:

```text
battle_server_app
config example
licenses
```

Do not include:

- source snapshot unless needed;
- build cache;
- secrets;
- frontend node_modules.

## 9. Deployment checklist

- [ ] Public config reviewed.
- [ ] Demo auth disabled or intentionally allowed.
- [ ] Telegram bot token configured through secret storage.
- [ ] WSS endpoint reachable.
- [ ] Mini App frontend reachable over HTTPS.
- [ ] TCP port reachable for Qt client.
- [ ] Metrics/logs visible.
- [ ] Load smoke test passes.
- [ ] Security smoke tests pass.
