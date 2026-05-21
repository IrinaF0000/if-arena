# Arena Objects

## Hero

Player-controlled unit.

Core fields:

```text
id
playerId
team
position
velocity/intent
hp/maxHp
attackCooldown
dashCooldown
isCarrier
```

MVP stats should be equal for both players.

## Objective: objective

Central object used by Objective Run.

States:

```text
center
carried
dropped
respawning
```

Core fields:

```text
state
position
carrierUnitId, optional
lastCarrierUnitId, optional
pickupLockedUntilTick, optional
spawnPosition
```

Rules:

- starts at map center;
- can be picked up automatically when the server validates proximity and pickup lock;
- carrier movement is slowed;
- drops on successful hit against carrier;
- capture occurs in carrier's own base zone;
- short pickup lock prevents immediate re-pickup after drop.

## Base zone

Scoring area for each player.

Requirements:

- symmetric placement;
- clear visual border;
- enough radius to capture without pixel-perfect positioning;
- canonical positions transformed by clients for local player view.

## Mine

Static hazard.

Rules:

- visible or clearly telegraphed in MVP;
- triggers when hero enters radius;
- server applies damage/drop effects;
- can be single-use or respawning by config.

## Tower

Static area-control hazard.

Rules:

- chooses target deterministically;
- has range and cooldown;
- attacks both players equally;
- should be visible with clear range indication if possible.

## Drone/Crow

Moving neutral hazard, stretch for MVP.

Rules:

- deterministic movement path or simple nearest-player pressure;
- not a full third-party player;
- speed and damage must be tuned to avoid chaos;
- path should be symmetric or centered.

## Wall/Obstacle

Grid-authored blocking object.

Rules:

- aligned to logical cells;
- symmetric under 180-degree rotation;
- collision uses world coordinates and hero radius.
