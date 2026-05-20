# Game Design

## Design intent

IF Arena is a small playable real-time arena game built to demonstrate networking and backend engineering. The gameplay should be simple enough to implement well, but expressive enough to show state synchronization, server authority, hazards, objective state, player-oriented rendering, and fast user input.

## Core loop

```text
connect -> join match -> move/aim/attack -> contest objective -> carry/drop/capture -> score -> rematch
```

The player should understand the basic loop quickly:

- move with WASD or touch joystick;
- aim with mouse or local target direction;
- attack with left mouse button, Space, or action button;
- interact with the objective using E or touch button;
- avoid mines and tower ranges;
- use hazards and terrain to pressure the enemy;
- carry the objective back to own base to score.

## MVP mode: Objective Run

Two players fight over a central energy objective.

Rules:

- the objective spawns at the center of the arena;
- a player can pick it up by standing near it and interacting;
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

## Player unit

MVP uses one hero per player.

Initial capabilities:

- smooth movement;
- primary aimed attack;
- dash/dodge with cooldown;
- interact with objective.

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
