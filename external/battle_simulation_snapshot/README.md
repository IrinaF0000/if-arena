# battle_simulation_snapshot

Read-only copy of the existing local battle simulation project.

## Snapshot record

- Imported path: `external/battle_simulation_snapshot/battle_sim/`
- Imported date: 2026-05-19
- Source: local `battle_simulation` project copy; the imported README references `https://github.com/IrinaF0000/battle_simulation`
- Source commit: unavailable in this snapshot because `.git` metadata was not copied
- Cleanup check: no copied build directories, CMake generated files, or binary/object outputs found during the 0002 import verification

Current top-level shape:

```text
external/battle_simulation_snapshot/
  README.md
  AGENTS.md
  battle_sim/
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
