# Core tests

Add tests proving that `src/battle_core/` preserves deterministic behavior after extraction from `external/battle_simulation_snapshot/`.

Minimum first test:

```text
reference scenario -> extracted battle_core -> trace/snapshot -> compare with known baseline
```
