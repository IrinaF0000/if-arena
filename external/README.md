# External Project Snapshots

This directory is reserved for source snapshots or third-party code used as input material for the new networking project.

## Current planned snapshot

- `battle_simulation_snapshot/` - an empty placeholder for the existing local battle simulation project.

The original repository should remain untouched. When the project starts, copy the old project source into `battle_simulation_snapshot/` as a read-only reference, then extract the reusable gameplay code into `src/battle_core/`.

Do not make the TCP server, Qt client, or protocol depend directly on this snapshot directory. The snapshot exists only to support controlled extraction and comparison.
