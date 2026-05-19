# Balancing Notes

This is not a competitive game. Balance exists to make the demo playable, fair, and understandable.

## Initial targets

- Arena size: 21 x 13 logical cells.
- Match duration: 2-4 minutes.
- Score limit: first to 3 captures.
- Player HP: enough to survive several mistakes.
- Normal movement speed: configurable.
- Carrier movement speed: about 80% of normal speed.
- Attack cooldown: short enough to feel responsive, long enough to make timing visible.
- Pickup lock after drop: about 0.5 seconds.
- Tower damage: meaningful but not instantly fatal.
- Mine damage: punishing but avoidable.
- Drone/crow speed: slower than player movement or clearly predictable.

## Tuning rules

Prefer changing configuration over code:

- hp;
- movement speed;
- carrier speed multiplier;
- attack cooldown;
- damage;
- pickup lock duration;
- tower range;
- mine radius;
- objective score limit;
- match duration.

## Fairness rules

- Both players start with the same stats.
- The map must use 180-degree rotational symmetry.
- Hazards and obstacles must be symmetric.
- Route lengths and route risks should be comparable.
- The local player-oriented view must not affect server-side rules.

## Avoid

- random unavoidable damage;
- invisible hazards in MVP;
- snowball mechanics;
- permanent upgrades;
- too many simultaneous abilities;
- maps too large to find the opponent;
- maps too small to dodge hazards;
- immediate pickup loops after carrier is hit.

## Useful playtest questions

- Can a new player understand what to do in 30 seconds?
- Can the player tell what is dangerous?
- Does the objective force interaction?
- Can a carrier be caught, but not too easily?
- Does the pickup lock feel clear and fair?
- Are hazards useful for strategy rather than just annoyance?
- Is the match short enough to replay quickly?
