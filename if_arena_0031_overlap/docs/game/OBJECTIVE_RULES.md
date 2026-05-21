# Objective Rules: Objective Run

## Goal

Players compete for a central objective. A player scores by carrying it to their own base.

Default win condition:

```text
first to 3 captures wins
```

## Objective lifecycle

```text
center spawn -> pickup -> carried -> drop or capture -> respawn/reset
```

## Pickup

- Player must be within pickup radius.
- Pickup is automatic when the server validates distance, pickup lock, and current objective state.
- The client does not need a separate `interact` command for MVP pickup.
- Client cannot claim ownership of the objective.

## Carrying

- Carrier is visible to both players.
- Carrier movement speed is reduced, initially to 80% of normal speed.
- Dash may be allowed with normal or increased cooldown, or disabled while carrying if balancing requires it.

## Drop

The carrier drops the objective when:

- hit by the opposing player;
- defeated;
- hit by a configured hazard, if enabled.

The server decides whether a hit landed. The client never decides objective drop.

## Pickup lock

After a drop, apply a short pickup lock:

```text
initial value: 0.5 seconds
```

Purpose:

- prevent immediate re-pickup by the same carrier;
- create a visible contest moment;
- avoid noisy drop/pickup loops.

## Capture

A capture happens automatically when:

- the player is the carrier;
- the carrier enters their own base zone;
- the match is active;
- the server validates the capture.

After capture:

- increment score;
- emit objective captured event;
- reset or respawn the objective at center after configurable delay;
- end match if score limit reached.
