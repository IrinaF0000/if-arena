# Repository Bootstrap

## Step 1 - Copy old project snapshot

Copy the existing local battle simulation project into:

```text
external/battle_simulation_snapshot/
```

This is a read-only reference. Do not develop the new server or Qt client inside this directory.

## Step 2 - Build empty skeleton

```bash
cmake -S . -B build -DBATTLE_BUILD_QT_CLIENT=OFF
cmake --build build
```

The initial skeleton builds placeholder targets only.

## Step 3 - Extract battle_core

Follow:

- `docs/project/ORIGINAL_PROJECT_IMPORT.md`
- `docs/project/CORE_EXTRACTION_CHECKLIST.md`

Target result:

```text
battle_core = static library extracted from the old simulation
```

## Step 4 - Add networking

Only after the core boundary is clean, start implementing:

```text
src/battle_protocol/
src/battle_backend/
src/battle_transport_tcp/
src/battle_transport_ws/
src/battle_server_app/
src/battle_cli_client/
tools/load_client/
src/battle_qt_client/
```
