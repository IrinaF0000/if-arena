# Qt UI

Qt Widgets UI for the local playable client.

- `MainWindow` owns connection controls, create/join lobby controls, HUD labels, error surface, event log, and keyboard input.
- `ArenaView` paints authoritative snapshots only: map grid, local/enemy base bands, objective, hazards, SVG player sprites, outlines, HP bars, and local/enemy labels.
- Keyboard controls send intentions through `NetworkClient`: W/A/S/D or arrows move, Space attacks, Shift dashes, and Esc disconnects.
- Mouse movement updates aim only; left click attacks and right click dashes toward the current aim.
- Objective pickup and capture are automatic server rules and are shown through snapshots/events.
- Player sprites are embedded Qt resources from `assets/players/`; the asset is rendering-only and does not define gameplay.

The UI never mutates authoritative positions, HP, score, objective state, or match result.
