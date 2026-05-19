# Telegram Mini App

## Purpose

The Telegram Mini App is an optional web/mobile client for IF Arena. It connects to the same authoritative backend as the Qt client, but it uses WebSocket instead of raw TCP.

## Why WebSocket

Telegram Mini App runs in a WebView/browser-like environment. Browser clients do not open arbitrary raw TCP sockets. They use Web APIs such as WebSocket for bidirectional communication.

Therefore:

```text
Qt client -> raw TCP -> backend
Telegram Mini App -> WebSocket/WSS -> same backend
```

## UX goals

- Join a match quickly from Telegram.
- Support invite flow through match code or start parameter.
- Provide touch controls suitable for mobile:
  - virtual joystick or D-pad;
  - attack;
  - dash;
  - interact/capture.
- Provide desktop controls when opened in desktop Telegram:
  - WASD;
  - mouse aim;
  - click/Space attack;
  - Shift dash;
  - E interact.
- Clearly show:
  - connection state;
  - player team;
  - HP;
  - cooldowns;
  - score/objective status;
  - match result.

## Frontend responsibilities

The Mini App may:

- read `window.Telegram.WebApp.initData`;
- send raw `initData` to backend;
- render game snapshots;
- collect player input;
- send input commands;
- show errors and reconnect state.

The Mini App must not:

- validate Telegram identity locally as authority;
- trust `initDataUnsafe`;
- store bot token or server secrets;
- decide whether damage, movement, scoring, or win/loss happened;
- contain authoritative game rules.

## Backend responsibilities

The backend must:

- validate raw `initData`;
- create an authenticated player session;
- reject invalid or stale Telegram init data;
- enforce all protocol limits;
- rate-limit input;
- keep server authority over game state;
- redact sensitive data from logs.

## Launch flow

```text
User opens bot
  -> taps Play
  -> Telegram opens Mini App URL
  -> Mini App loads
  -> Mini App reads initData
  -> Mini App opens WebSocket
  -> Mini App sends auth_request(mode=telegram, initData=rawInitData)
  -> backend validates initData
  -> backend sends auth_result
  -> user creates or joins match
```

## Local development flow

For local development, use a non-Telegram demo auth mode only on localhost/dev config.

Do not allow demo auth on public production config unless explicitly intended for a public demo with limited permissions.

## Deployment notes

Public Telegram Mini App requires:

- HTTPS hosting for frontend;
- WSS endpoint for backend gateway;
- configured bot button/menu;
- no secrets committed to frontend;
- environment-based backend endpoint config.

## References

- Telegram Mini Apps: https://core.telegram.org/bots/webapps
- WebSocket API: https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API
