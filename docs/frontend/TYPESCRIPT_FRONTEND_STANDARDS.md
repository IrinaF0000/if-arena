# TypeScript Frontend Standards

These standards apply to `frontend/telegram_mini_app/`.

## Language and tooling

- TypeScript strict mode.
- ESLint enabled.
- Prettier enabled.
- No unchecked `any`.
- Prefer explicit domain types.
- Treat network input as `unknown` before validation.
- Avoid large framework complexity in MVP. Canvas + small modules are enough.

## Architecture

Recommended module boundaries:

```text
src/telegram/
  TelegramBridge.ts
  Reads Telegram WebApp context and returns raw initData.

src/network/
  WebSocketClient.ts
  Owns connection, reconnect state, send queue, and message dispatch.

src/protocol/
  ProtocolTypes.ts
  Transport-independent DTO shape used by frontend.

src/game/
  ArenaState.ts
  Client-side presentation state only.

src/ui/
  CanvasRenderer.ts
  TouchControls.ts
  Hud.ts
```

## Forbidden

- No authoritative game rules.
- No bot token.
- No trusting Telegram `initDataUnsafe`.
- No mutation of server-owned state except local presentation cache.
- No raw `innerHTML` from server data.
- No silent catch blocks.
- No unbounded client-side message queues.

## Runtime validation

Inbound WebSocket messages must be validated before use. For MVP, hand-written type guards are acceptable. Later, a schema library can be used.

Example:

```ts
function isSnapshotMessage(value: unknown): value is SnapshotMessage {
  // validate object, version, type, and payload shape
}
```

## Token-saving rule for agents

When editing frontend files, agents should inspect only relevant modules and avoid reading the entire generated dependency tree. Never paste `node_modules` or built assets into prompts.
