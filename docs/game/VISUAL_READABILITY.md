# Visual Readability

## Principles

- The player should understand the arena state at a glance.
- Normal play should look like a compact arena, not a debug grid.
- Do not rely on color alone.
- Local player view should make the player's own base feel like the bottom/home side.
- The central objective and carrier must always be obvious.
- Attack range, attack direction, hit feedback, dash feedback, and base zones must be visible enough to understand play without reading logs.

## Arena view

- Keep the full arena visible, but avoid unnecessary horizontal stretch.
- The game should read as bottom-to-top objective play.
- The logical grid remains for map authoring and collision.
- Do not render a strong chessboard/debug grid in normal play.
- If floor/grid texture is used, keep it subtle.
- Static objects should visibly align to the logical grid: bases, walls, hazards, spawns, objective spawn, dropped objective.
- Players move smoothly and should not visually snap to cell centers.

## Suggested visual language

| Object | Color in player view | Shape/icon | Extra cue |
|---|---|---|---|
| Own hero | Blue/cyan | Circle | Bright outline + aim/facing cue |
| Enemy hero | Red | Circle | Enemy outline |
| Objective | Yellow/white | Star/objective | Center platform/ring + glow/label |
| Carrier | Team color + glow | Circle with objective marker | Halo/trail/objective icon |
| Mine | Orange | Diamond | Warning outline |
| Tower | Gray/purple | Square/turret | Range hint |
| Drone/Crow | Purple/dark | Triangle/bird/drone | Movement trail |
| Own base | Blue/cyan zone | Rect/circle zone | Base label |
| Enemy base | Red zone | Rect/circle zone | Base label |
| Wall | Dark neutral | Block/tile | Grid aligned |

Current post-0054 palette uses a charcoal tactical board, muted gold grid/borders, cyan own-player accents, coral enemy accents, amber objective highlights, orange mines, violet towers, and pale crow markers. This keeps the arena dark enough for contrast without making every UI surface the same blue/slate family.

## Player indicators

Use only indicators that are immediately meaningful:

- HP ring/bar if HP affects combat;
- facing/aim marker if aim affects attack or dash;
- carrier marker if the player carries the objective;
- cooldown feedback near HUD or action indicator.

Remove unexplained colored sticks/bars above players from normal play. In the Qt MVP view, HP is shown as a green ring around the player and carrier state is shown as a yellow halo/objective marker, so the space above the player is reserved for meaningful objective feedback rather than debug bars.

## Combat feedback

Show at least minimal feedback for:

- local attack range or attack direction preview;
- attack fired;
- hit landed or missed, where feasible;
- dash direction/trail;
- ability on cooldown;
- carrier hit and objective drop.

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
- unclear carrier state;
- debug-only visuals in normal play.

## Asset constraints

Use original simple shapes or generated placeholder assets only. Do not copy commercial game assets, maps, icons, names, or UI identity.
