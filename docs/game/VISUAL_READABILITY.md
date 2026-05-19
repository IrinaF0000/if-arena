# Visual Readability

## Principles

- The player should understand the arena state at a glance.
- Do not rely on color alone.
- Local player view should make the player's own base feel like the bottom/home side.
- The central objective and carrier must always be obvious.

## Suggested visual language

| Object | Color in player view | Shape/icon | Extra cue |
|---|---|---|---|
| Own hero | Blue/cyan | Circle | Bright outline |
| Enemy hero | Red | Circle | Enemy outline |
| Objective | Yellow/white | Star/objective | Glow/label |
| Carrier | Team color + glow | Circle with objective marker | Trail/halo |
| Mine | Orange | Diamond | Warning outline |
| Tower | Gray/purple | Square/turret | Range hint |
| Drone/Crow | Purple/dark | Triangle/bird/drone | Movement trail |
| Own base | Blue/cyan zone | Rect/circle zone | Base label |
| Enemy base | Red zone | Rect/circle zone | Base label |
| Wall | Dark neutral | Block/tile | Grid aligned |

## Events that need feedback

- objective pickup;
- objective drop;
- pickup lock active;
- objective captured;
- hit landed;
- attack on cooldown;
- carrier slowed;
- match point / win/loss.

## Accessibility

Use:

- shape differences;
- outlines;
- icons;
- text labels where useful;
- consistent placement of HUD elements.

Avoid:

- only red/green distinction;
- tiny indicators;
- overloaded particle effects;
- unclear carrier state.

## Portfolio asset constraints

Use original simple shapes or generated placeholder assets only. Do not copy commercial game assets, maps, icons, names, or UI identity.
