# scenarios

Battle scenario definitions used by normal game startup.

`arena_small_objective_run.json` is the source of truth for the playable default arena. `battle_core` receives parsed value-type config only; filesystem and JSON parsing stay in backend/server-app layers.

Snapshot object semantics are derived from this config and documented in `docs/architecture/PROTOCOL.md`. Clients consume server-authored metadata such as `visualId`, `blocksMovement`, `damage`, `causesDrop`, `rangeRadius`, cooldown, and neutral/team ownership; they must not duplicate map or hazard rules locally.
