# Original Project Import

## Goal

Use the existing local `battle_simulation` project as source material without modifying the original repository.

## Current import status

The original project snapshot is present at:

```text
external/battle_simulation_snapshot/battle_sim/
```

It was recorded on 2026-05-19 from a local `battle_simulation` project copy. The imported project README references `https://github.com/IrinaF0000/battle_simulation`; exact source commit metadata is unavailable because `.git` metadata was not copied.

## Recommended approach

1. Create a new repository from this skeleton.
2. Copy the old project into `external/battle_simulation_snapshot/`.
3. Treat that directory as read-only reference.
4. Extract selected code into `src/battle_core/`.
5. Build all new networking, Qt, server, and load-test functionality outside the snapshot.

## Why not use the old executable?

The server needs direct access to simulation state, ticks, snapshots, events, validation, and multiple matches. Calling an old `.exe` through files or stdin/stdout would be brittle and unsuitable for real-time high-load TCP behavior.

## Why static library first?

`battle_core` should be a static library for simplicity:

- easier CMake setup;
- no DLL export/import complexity;
- easier cross-platform development;
- direct linking from server and tests;
- no ABI boundary concerns for a portfolio project.

A DLL/shared library can be considered later only if there is a clear reason.

## Snapshot cleanup

Do not commit:

- build directories;
- generated binaries;
- `.exe`, `.dll`, `.lib`, `.obj`, `.pdb` unless explicitly justified;
- IDE caches;
- temporary files;
- personal machine paths.

## Extraction rule

Do not refactor and network-enable the snapshot in place. Copy/adapt code into `src/battle_core/` and add tests there.
