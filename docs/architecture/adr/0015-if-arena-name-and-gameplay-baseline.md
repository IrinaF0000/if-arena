# ADR 0015: IF Arena Name and Gameplay Baseline

## Status

Accepted

## Context

The project needs a name that is safe for public use and does not resemble a commercial game platform. The project also needs a stable MVP gameplay target so agents do not drift into unrelated game designs.

## Decision

The project is named **IF Arena**.

The name is written as `IF Arena`, not `If Arena`, so it remains a proper project name rather than the incomplete English phrase "if arena".

The MVP gameplay baseline is Objective Run:

- 2-player real-time objective arena;
- 21 x 13 logical-cell map;
- smooth movement on a grid-authored arena;
- 180-degree rotational symmetry around the central objective;
- own base at bottom in each player's local view;
- first to 3 captures wins;
- carrier slowdown;
- objective drop on successful hit;
- short pickup lock after drop;
- neutral server-controlled hazards.

## Consequences

- All docs and frontend labels should use `IF Arena`.
- Agents must not introduce alternative project names without an ADR update.
- Agents must treat Objective Run as the MVP mode unless the task explicitly changes gameplay scope.
- Gameplay changes affecting fairness, objective rules, or local orientation require architecture/gameplay review.
