# battle_qt_client

Qt Widgets desktop client over raw TCP.

Owns:

- connection screen;
- lobby;
- player-oriented arena rendering;
- keyboard/mouse controls;
- HUD for HP, cooldowns, score, objective state, carrier state, and connection state;
- local coordinate transforms from canonical world snapshots to player view and back.

Must not link to `battle_core`.
Must not own authoritative game rules.
Network code must stay separated from widgets.
Do not block the UI thread.

Current network skeleton:

- `network/NetworkClient.hpp` defines connection state, endpoint, and intent send gating.
- The skeleton has no socket ownership and no Qt includes yet, so it can build without a configured Qt package.
- A later Qt adapter should wrap this shape with `QObject` signals/slots and `QTcpSocket`.
