# Qt Client Manual Checklist

## Setup

- [ ] Add Qt tools to PATH: `$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"`.
- [ ] Build with Qt enabled: `cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"`.
- [ ] Run `cmake --build build-qt-mingw --parallel`.
- [ ] Run `ctest --test-dir build-qt-mingw --output-on-failure`.
- [ ] Build succeeds and `battle_qt_client` links to Qt Widgets/Network and `battle_protocol`, not `battle_core`.
- [ ] Start local server: `build/battle_server_app --config config/examples/server.local.json --max-clients 2`.
- [ ] Launch two clients: `build-qt-mingw\battle_qt_client.exe`.

## Connection And Lobby

- [ ] Initial connection state is `disconnected`; Create and Join are disabled.
- [ ] Connect to `127.0.0.1:5555`; state moves through connecting, connected, authenticated.
- [ ] First client creates a match; match code appears in the Join field and event log.
- [ ] Second client connects, joins with that match code, and both clients enter `in match`.
- [ ] Disconnect button and Esc close the TCP session without freezing the UI.
- [ ] Reconnect works after disconnect without restarting the client.

## Rendering And HUD

- [ ] Arena grid is visible and scaled without clipping.
- [ ] Own base appears at the bottom and enemy base at the top for both clients.
- [ ] Own hero is cyan/blue with an outline and `YOU` label.
- [ ] Enemy hero is red with a non-color label.
- [ ] Objective is visible at center/start and shows its state label.
- [ ] Hazards are distinct from players/objective by shape and color.
- [ ] HUD shows connection, session/match, HP, attack cooldown, dash cooldown, score, objective state, tick, and snapshot age.
- [ ] Event log shows auth, join, input acks/rejections, server events, and errors.

## Controls

- [ ] W/A/S/D and arrow keys move relative to the local player view.
- [ ] Releasing all movement keys sends stop and the hero stops after the authoritative snapshot updates.
- [ ] Mouse movement aims without moving the hero.
- [ ] Left click and Space send attack intentions.
- [ ] Shift sends dash intention.
- [ ] E sends interact intention near objective/base.
- [ ] Rejected or rate-limited input appears as feedback and does not locally alter HP, score, position, or objective state.

## Responsiveness And Failure

- [ ] UI remains responsive during connect, create/join, play, disconnect, and server shutdown.
- [ ] Stopping the server shows a user-facing socket error or disconnected state.
- [ ] Invalid match code shows a backend rejection and leaves the client usable.
- [ ] No raw Telegram init data, secrets, or unbounded payloads are displayed.
