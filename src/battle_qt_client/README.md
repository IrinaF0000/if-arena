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

Current implementation:

- `QtClientMain.cpp` starts a Qt Widgets desktop client.
- `network/NetworkClient.*` owns the asynchronous `QTcpSocket`, length-prefixed TCP framing, protocol envelope validation, demo auth, create/join, ping/pong, and intention sending.
- `ui/MainWindow.*` owns the connection/lobby controls, HUD, event log, and keyboard commands.
- `ui/ArenaView.*` renders only validated authoritative snapshots and converts mouse input into local intention directions.
- `game/CoordinateTransform.*` contains the player-oriented view helpers covered by `battle_qt_client_transform_tests`.

Build on Windows with the Qt MinGW kit:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"
cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
cmake --build build-qt-mingw --parallel
ctest --test-dir build-qt-mingw --output-on-failure
build-qt-mingw\battle_qt_client.exe
```

The default repository build keeps `BATTLE_BUILD_QT_CLIENT=OFF` so non-Qt CI environments still compile the backend, transports, CLI, load client, and tests. The Qt target links to `battle_protocol` and Qt Widgets/Network; it does not link to `battle_core`.
