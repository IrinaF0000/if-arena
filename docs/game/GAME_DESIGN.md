# Game Design

## Design intent

IF Arena is a small playable real-time arena game built to implement networking and backend engineering. The gameplay should be simple enough to implement well, but expressive enough to show state synchronization, server authority, hazards, objective state, player-oriented rendering, and fast user input.

## Core loop

```text
connect -> join match -> move/aim/attack -> contest objective -> carry/drop/capture -> score -> rematch
```

The player should understand the basic loop quickly:

- move with WASD, arrow keys, or touch joystick;
- aim with mouse or local target direction;
- attack with left mouse button, Space, or action button;
- dash with right mouse button, Shift, or dash button;
- pick up and capture the objective automatically by reaching it or the base;
- avoid mines and tower ranges;
- use hazards and terrain to pressure the enemy;
- carry the objective back to own base to score.

## MVP mode: Objective Run

Two players fight over a central energy objective.

Rules:

- the objective spawns at the center of the arena;
- a player picks it up automatically when close enough and not pickup-locked;
- the carrier is visible to both clients;
- the carrier moves slower than normal;
- the carrier scores by reaching their own base zone;
- the first player to 3 captures wins;
- the match can also end after a configurable time limit;
- the carrier drops the objective on a successful hit;
- server-controlled hazards may also force a drop if configured;
- after a drop, a short pickup lock prevents instant re-pickup by the same player.

Alternative earlier debug mode:

- `Duel With Hazards`: first player to 3 knockouts wins.

This mode is only for debugging and should not replace Objective Run as the MVP gameplay target.


## Controls

Primary desktop controls:

- WASD or arrow keys move the player;
- mouse movement updates aim only and must not move, dash, attack, teleport, or spam commands;
- left mouse button or Space attacks toward aim;
- right mouse button or Shift dashes toward aim, falling back to movement direction if aim is unavailable;
- objective pickup and capture are automatic server-side rules.

The client may send aim updates, but they must be throttled and meaningful. Idle clients must not send repeated no-op gameplay commands.

## Visual readability

Normal play should look like a compact arena, not a debug grid. The logical grid remains for map authoring and collision, but the visible grid must be subtle or hidden. Static objects such as bases, walls, hazards, spawns, objective spawn, and dropped objective should align clearly to the logical grid. Players move smoothly and should not visually snap to cells.

The arena should read as a bottom-to-top objective game. Avoid unnecessary horizontal stretch. The objective, carrier state, attack range/direction, hit feedback, dash feedback, and bases must be visible without reading logs. Unclear player indicators such as unexplained sticks or bars should be removed or replaced with documented indicators such as HP, facing/aim, or carrier markers.

## Player unit

MVP uses one hero per player.

Initial capabilities:

- smooth movement;
- primary aimed attack;
- dash/dodge with cooldown;
- automatic objective pickup/capture.

Non-goals for MVP:

- many classes;
- item builds;
- persistent progression;
- complex abilities;
- AI teammates.

## Map and fairness

The MVP arena should be authored as a 21 x 13 logical-cell map.

Requirements:

- the center cell contains the initial objective;
- the layout uses 180-degree rotational symmetry around the center;
- both players have identical stats and start conditions;
- bases, spawn points, obstacles, hazards, route lengths, and route risks are symmetric;
- the full arena should fit on screen;
- movement is smooth, but walls, bases, mines, towers, and objective placement are aligned to the logical grid.

## Player-oriented view

Each player should see their own base at the bottom of the screen and the enemy base at the top. The server keeps canonical world coordinates. Clients transform world coordinates to local screen coordinates and transform local input back into canonical world commands before sending them to the server.

Canonical replay/spectator behavior is defined in `docs/architecture/TECHNICAL_ARCHITECTURE.md`.

## Hazards

Hazards should create positioning decisions:

- tower controls an area;
- mine punishes careless routes;
- drone/crow creates moving pressure.

Hazards must be deterministic and server-controlled. They should create tactical pressure, not behave like a full third-party player.

## Match duration

Target duration:

- ideal: 2-4 minutes;
- local debug mode may be shorter;
- load-test mode may use simplified shorter matches.

## Skill expression

The game should reward:

- movement;
- aiming;
- cooldown timing;
- dodging;
- objective timing;
- using hazards and terrain;
- predicting the enemy path after a objective pickup.

## Fairness rules

- Both players start with equal capabilities.
- No permanent upgrades.
- No pay-to-win mechanics.
- No hidden server-side randomness unless seed is recorded.
- Hazards must be visible or clearly telegraphed before damage where possible.
- Carrier slowdown and pickup lock must apply equally to both players.

## IP and originality constraint

The game may use generic arena patterns but must not copy proprietary assets, character names, maps, UI, exact rule combinations, or distinctive game identity from existing commercial games.
