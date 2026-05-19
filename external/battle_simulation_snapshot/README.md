# battle_simulation_snapshot

Empty placeholder for the existing local battle simulation project.

Copy the old project here when you start implementation:

```text
external/battle_simulation_snapshot/
  CMakeLists.txt
  src/
  docs/
  tests/
  tools/
  ...
```

Rules:

1. Treat this directory as a read-only source snapshot.
2. Do not add TCP, Qt, server, load-testing, or deployment code here.
3. Do not make new code depend directly on this directory in the final architecture.
4. Extract reusable gameplay logic into `src/battle_core/`.
5. Preserve a reference scenario so the extracted core can be compared against the original behavior.

Recommended workflow:

```text
old battle_simulation repository
        |
        | copy once
        v
external/battle_simulation_snapshot/        read-only reference
        |
        | controlled extraction
        v
src/battle_core/                            reusable static library
```
