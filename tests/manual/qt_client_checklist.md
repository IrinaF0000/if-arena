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

- [ ] Arena floor/grid is subtle, scaled without clipping, and no longer reads like a debug grid.
- [ ] Arena view keeps the full map visible while emphasizing bottom-to-top objective play.
- [ ] Own base appears at the bottom and enemy base at the top for both clients.
- [ ] Own hero is cyan/blue with an outline and `YOU` label.
- [ ] Enemy hero is red with a non-color label.
- [ ] Objective is visible at center/start or dropped position with ring/glow and state label.
- [ ] Carrier state is obvious through a halo/objective marker on the carrying player.
- [ ] Hazards are distinct from players/objective by shape and color.
- [ ] HUD shows connection, session/match, HP, attack cooldown, dash cooldown, score, objective state, tick, and snapshot age.
- [ ] Compact controls hint is visible above the arena.
- [ ] Event log shows auth, join, rejections, server events, ping, and errors without repeated accepted idle input spam.

## Controls

- [ ] W/A/S/D and arrow keys move relative to the local player view.
- [ ] Releasing all movement keys sends stop and the hero stops after the authoritative snapshot updates.
- [ ] Mouse movement aims only; it does not move, dash, attack, teleport, or spam gameplay commands.
- [ ] Left click and Space attack toward aim direction and show range/direction feedback.
- [ ] Right click and Shift dash toward aim direction, falling back to movement direction, and show dash feedback.
- [ ] Objective pickup and capture remain automatic; no `E`/interact input is required for the MVP objective flow.
- [ ] Rejected or rate-limited input appears as feedback and does not locally alter HP, score, position, or objective state.

## Live Match Stability

- [ ] Match remains responsive after 60 seconds of play.
- [ ] Tick continues beyond `1200` without mouse movement or idle command spam.
- [ ] Movement submitted after tick `1200` changes the authoritative player position.

## Responsiveness And Failure

- [ ] UI remains responsive during connect, create/join, play, disconnect, and server shutdown.
- [ ] Stopping the server shows a user-facing socket error or disconnected state.
- [ ] Invalid match code shows a backend rejection and leaves the client usable.
- [ ] No raw Telegram init data, secrets, or unbounded payloads are displayed.
