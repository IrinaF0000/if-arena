# ADR 0007: Server-controlled Neutral Hazards

## Status

Accepted

## Context

The original simulation already has tick-based automated behavior. The networking project should reuse that strength while keeping gameplay simple for human players.

## Decision

Neutral hazards such as towers, mines, and drones/crows will be server-controlled deterministic objects. Human players control only their own hero units.

## Consequences

Positive:

- reuses existing simulation concepts;
- demonstrates tick-based server logic;
- adds interesting gameplay without complex AI opponents;
- improves replay/debug value.

Negative:

- hazards must be visually clear to avoid feeling unfair;
- balance requires tuning;
- server snapshots need to include hazard state.
