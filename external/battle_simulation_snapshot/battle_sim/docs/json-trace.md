# JSON Trace

The CLI can write an optional deterministic JSON event trace while keeping legacy stdout unchanged:

```bash
./build/battle_sim commands_example.txt --trace-json trace.json
./build/battle_sim commands_example.txt config/default.cfg --trace-json trace.json
```

Trace summary:

```bash
./build/battle_sim inspect trace.json
```

Visual replay:

```text
Open tools/replay-viewer/index.html and load trace.json.
```

Local browser run and replay:

```bash
python tools/local-runner/server.py --exe build/battle_sim
```

Then open `http://127.0.0.1:8765`. The local runner generates a scenario command file, runs `battle_sim <commands-file> --trace-json <trace-file>`, and loads the resulting trace in the viewer.

The trace is a JSON array. Each record contains `tick`, `event`, and event-specific fields. Combat actions are emitted explicitly; consumers should not infer attacks or healing from hit-point changes.

Small trace excerpt:

```json
[
  {
    "tick": 1,
    "event": "MAP_CREATED",
    "width": 8,
    "height": 5
  },
  {
    "tick": 1,
    "event": "UNIT_SPAWNED",
    "unitId": 1,
    "unitType": "Swordsman",
    "x": 0,
    "y": 2
  }
]
```

Current event names:

- `MAP_CREATED`
- `UNIT_SPAWNED`
- `MARCH_STARTED`
- `MARCH_ENDED`
- `UNIT_MOVED`
- `UNIT_ATTACKED`
- `UNIT_HEALED`
- `UNIT_DIED`

Combat visualization fields:

- `UNIT_MOVED`: `unitId`, `fromX`, `fromY`, `x`, `y`
- `UNIT_ATTACKED` and `UNIT_HEALED`: `sourceUnitId`, `targetUnitId`, `sourceX`, `sourceY`, `targetX`, `targetY`, `amount`, `targetHpBefore`, `targetHpAfter`, `resultingValue`
- `UNIT_DIED`: `unitId`, `x`, `y`, `reason`

The replay viewer uses these fields for current-tick movement, attack, healing, death, and explosion effects. Older traces with missing visualization fields still load; the viewer falls back to unit positions where possible.

The format is intended for inspection and regression tooling, not as a gameplay scripting API.
