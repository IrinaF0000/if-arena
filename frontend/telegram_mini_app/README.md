# IF Arena Telegram Mini App

Strict TypeScript frontend for the Telegram Mini App client.

Owns:

- Telegram bridge;
- WebSocket client;
- Canvas rendering;
- touch/desktop controls;
- local player-oriented coordinate transforms;
- HUD and connection state.

Must not contain:

- Telegram bot token;
- backend session secret;
- authoritative game rules;
- direct trust in `initDataUnsafe`.

The Mini App sends raw Telegram `initData` to the backend for validation and then sends only player intentions such as move, aim, attack, dash, and interact.
