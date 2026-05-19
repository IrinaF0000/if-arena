# Player-Oriented View

## Goal

Each player should feel that they are moving forward toward the center/enemy side.

Player view:

- own base is at the bottom;
- enemy base is at the top;
- forward movement feels consistent for both players.

## Server authority

The server uses canonical world coordinates only. It does not create separate maps or separate physics for each player.

The client transforms:

```text
world coordinates -> local screen coordinates
local input -> canonical world direction/command
```

## Input mapping

Example with Blue at canonical bottom and Red at canonical top:

```text
Blue presses W -> canonical direction up
Red presses W  -> canonical direction down
```

Both players see their hero move forward on their own screen.

## Rendering

MVP player view may use local colors:

- own hero: blue/cyan;
- enemy hero: red;
- neutral hazards: orange/gray/purple;
- objective: bright yellow/white.

Replay/debug/spectator view should use canonical orientation and may use fixed team colors.

## Tests

Coordinate transform tests must cover:

- world-to-screen for both players;
- screen-to-world input direction for both players;
- objective, base, spawn, and hazard positions;
- no change to authoritative server coordinates.
