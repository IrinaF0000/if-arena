# ADR 0006: Objective-based Arena MVP

## Status

Accepted

## Context

A pure deathmatch is easy to implement but can become repetitive and does not strongly demonstrate objective state synchronization. A compact objective-based mode creates clearer gameplay and richer server state without requiring many characters or complex content.

## Decision

The first complete playable mode is **Objective Run**.

Players compete for a central objective and score by carrying it to their own base. Neutral hazards create map pressure.

MVP rule choices:

- 2 human players;
- 21 x 13 logical-cell arena;
- 180-degree rotational symmetry around the center;
- one center objective spawn;
- first to 3 captures wins;
- carrier moves slower;
- carrier drops the objective on successful hit;
- short pickup lock prevents immediate re-pickup;
- hazards are server-controlled and symmetric;
- clients may render player-oriented views, but the server uses canonical coordinates.

## Consequences

Positive:

- more interesting than deathmatch;
- useful server-owned objective state;
- easy to show in Qt and Telegram UI;
- good for replay/debugging;
- demonstrates authoritative validation and synchronization.

Negative:

- requires more gameplay validation than deathmatch;
- needs careful balance around carrier speed, attack cooldown, and pickup lock;
- requires coordinate transform tests for player-oriented rendering.
