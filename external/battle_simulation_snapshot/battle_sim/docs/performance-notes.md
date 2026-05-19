# Performance Notes

This document records current performance characteristics without adding a benchmark harness.

## Current Runtime Shape

The CLI runtime is deterministic in structure and uses feature-owned battle systems:

- command files are parsed line by line before simulation starts;
- entity turns iterate in creation order;
- each acting entity copies and stable-sorts its action rules by priority;
- target selection scans ECS entities through battle selectors and spatial query helpers;
- movement checks neighboring cells and may use RNG to choose between equal candidates;
- dead-entity cleanup runs after each tick's entity turns.

For the current sandbox-sized scenarios this is intentionally simple and easy to inspect.

## Likely Hot Spots

The first places to measure before optimizing are:

- repeated per-entity rule sorting in action execution;
- target queries that scan visible entities;
- map occupancy and neighbor checks during movement;
- event formatting when large scenarios produce many text lines;
- data-driven archetype parsing if many JSON files are loaded at startup.

## Debug Output

Optional debug output is enabled through config:

```text
feature io.legacy debugSummary=true
```

Debug lines are written to stderr with a `[debug]` prefix. Legacy stdout event lines stay unchanged so golden-output checks remain stable.

Current debug summary:

```text
[debug] scenario.complete rngSeed=random archetypeFiles=0
```

## Measurement Plan

When scenarios grow, prefer adding focused measurements in this order:

1. scenario runtime wall-clock for fixed-seed command files;
2. event count and output bytes;
3. entity count and average active entities per tick;
4. target-query candidate counts;
5. movement candidate counts.

Keep benchmark inputs deterministic and separate from compatibility command files unless a task explicitly changes them.
