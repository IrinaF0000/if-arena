# Qt network

Network code stays separated from widgets.

Current skeleton:

- `NetworkClient.hpp` holds transport-neutral connection state and player-intention send gate.
- It intentionally avoids Qt includes until the project adds a real `QObject`/`QTcpSocket` target.
- It never owns authoritative game state; it only tracks connection state and whether an intent may be sent.

Future signal/slot plan:

- `connectRequested(ServerEndpoint)` from UI -> NetworkClient connect slot.
- `disconnectRequested()` from UI -> NetworkClient disconnect slot.
- `intentReady(ClientIntent)` from input mapper -> NetworkClient send slot.
- `connectionStateChanged(ConnectionState)` from NetworkClient -> UI/HUD.
- `protocolMessageReceived(...)` from NetworkClient -> protocol/UI adapter after validation.
- `networkError(QString)` from NetworkClient -> UI user-facing error surface.

No blocking calls are allowed on the UI thread.
