# Map and Fairness

## Recommended MVP size

The recommended MVP arena size is:

```text
21 x 13 logical cells
```

This gives enough room for a central objective, two bases, two to three route options, simple hazards, and short matches while keeping the entire arena visible on screen.

## Symmetry

The MVP map must use 180-degree rotational symmetry around the central objective.

This means that if Blue has an obstacle, mine, tower, route, or base feature at one position, Red has an equivalent feature after rotating the map by 180 degrees around the center.

## Canonical coordinates

The server stores one canonical coordinate system.

Recommended canonical MVP layout:

```text
width  = 21
height = 13
center = (10, 6)
red base/spawn near top
blue base/spawn near bottom
```

Player clients may rotate the view so that each player sees their own base at the bottom.

## Grid-authored, smooth movement

The arena is authored on a grid, but player movement is smooth.

Grid-aligned:

- walls;
- bases;
- objective spawn;
- mines;
- towers;
- initial spawns.

Smooth/world-space:

- hero position;
- dash movement;
- projectile/hit checks if added;
- collision radius;
- objective pickup radius.

## Visual grid

The player should not need to count cells. Use a subtle floor/tile pattern instead of a heavy chessboard grid.

The player should understand:

- where walls are;
- where the base is;
- where the objective is;
- where hazards are;
- where safe and dangerous routes are.

## Current playable layout

The default local Objective Run scenario keeps the vertical center lane open so both players can move directly from spawn toward the center objective and directly back toward their own base. Obstacles shape the side routes and symmetric hazards add contest pressure around the objective without overlapping spawns, bases, or the objective spawn.
