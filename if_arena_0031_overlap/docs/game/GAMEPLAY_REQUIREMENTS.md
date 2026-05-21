# Gameplay Requirements

## MVP mode: Objective Run

- Two human players.
- Short real-time objective match.
- One central objective objective.
- Players score by carrying the objective to their own base.
- First to 3 captures wins by default.
- Neutral hazards create pressure.
- Server controls all rules and outcomes.

## Match duration

Target:

```text
2-4 minutes
```

## Arena layout

- Recommended MVP size: 21 x 13 logical cells.
- Use a single center cell for the objective spawn.
- Use 180-degree rotational symmetry around the central objective.
- Both sides must have equal route lengths, obstacle pressure, hazard pressure, base sizes, and spawn distances.
- The whole arena should fit on screen in the MVP.

## Movement model

- The map is authored on a logical grid.
- Player movement is smooth and real-time, not strict tile-by-tile movement.
- Obstacles, bases, hazards, and objective spawn points are grid-aligned.
- The grid may be visible as a subtle floor/tile pattern, but gameplay should feel like a top-down arena.

## Objective rules

- The objective starts in the center.
- A player picks it up automatically when close enough and not pickup-locked.
- The carrier moves slower, initially 80% of normal speed.
- The carrier drops the objective on successful hit.
- The objective may also drop on configured hazard hit or defeat.
- After a drop, apply a short pickup lock, initially 0.5 seconds, to prevent instant re-pickup.
- A capture is scored when the carrier enters their own base zone.
- After capture, the objective respawns at center after a short delay.

## Player abilities

MVP:

- move;
- aim;
- attack;
- dash/dodge;
- automatic objective pickup/capture.

Do not add many classes or complex ability trees in MVP.

## Combat rules

- Primary attack must have cooldown.
- Primary attack must require aim or direction.
- Dodging should be possible through movement and optional dash.
- A successful hit on the carrier must trigger objective drop.
- The client never decides whether a hit landed; the server does.

## Player-oriented view

- Each player sees own base at the bottom and enemy base at the top.
- Clients transform rendering and input relative to local orientation.
- Server uses canonical coordinates only.
- Replay/debug/spectator views use canonical orientation.

## Hazards

MVP hazards:

- mine;
- tower;
- drone/crow as stretch.

Hazards must be deterministic, visible or clearly telegraphed, and server-controlled.

## Fairness

- No permanent progression.
- No pay-to-win mechanics.
- Same base capabilities for both players in MVP.
- Symmetric map, base, spawn, obstacle, and hazard placement.
- Server validates all commands.

## UX requirement

A new player should understand the basic goal and controls within 30 seconds. Normal play must expose objective, carrier, attack range/direction, and base state visually, not only in logs.
