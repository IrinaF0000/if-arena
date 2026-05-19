# Replay Viewer

`tools/replay-viewer/index.html` is a static browser tool for inspecting deterministic JSON traces from the battle simulator. It reconstructs the map, keeps a readable combat log, and renders current-tick movement, attack, healing, death, and explosion effects.

A copy is also published under `docs/replay-viewer/` so GitHub Pages can host a static project demo.

## Static Demo

The viewer can load a prepared demo trace from:

```text
traces/basic-battle.json
```

When hosted through GitHub Pages, this gives reviewers a browser-based visual replay without requiring a local build.

The static demo does not run the C++ executable. It only replays JSON traces that were generated beforehand.

## Manual Trace Loading

Generate a trace:

```bash
./build/battle_sim commands_example.txt --trace-json trace.json
```

Open `tools/replay-viewer/index.html` in a browser, load `trace.json`, then step through ticks or play the replay.

## Local Runs

The viewer can also connect to the local runner helper and launch the simulator from the browser UI.

Build the simulator:

```bash
cmake -S . -B build
cmake --build build --parallel
```

Start the helper:

```bash
python tools/local-runner/server.py --exe build/battle_sim
```

Then open:

```text
http://127.0.0.1:8765
```

Open the viewer through the helper URL for local runs. Direct `file://` opening is still supported for manual `trace.json` loading only.

Run artifacts are written under `tools/local-runner/runs/` and include the generated scenario, stdout, stderr, exit code, and trace.

## Supported Events

- `MAP_CREATED`
- `UNIT_SPAWNED`
- `MARCH_STARTED`
- `MARCH_ENDED`
- `UNIT_MOVED`
- `UNIT_ATTACKED`
- `UNIT_HEALED`
- `UNIT_DIED`

Unsupported events stay visible in the event log and are skipped by the map replay.

Combat effects come from explicit trace events, not inferred hit-point deltas. Attack and heal records include source/target coordinates plus target HP before and after; move records include `fromX/fromY`; death records include position and reason.

## Scope

The viewer does not simulate gameplay rules. It reconstructs visible state from trace events, highlights current-tick actions, shows unit details, and reports malformed or incomplete events.
