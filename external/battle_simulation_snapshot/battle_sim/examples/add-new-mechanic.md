# Add a New Mechanic

This example shows how a small unit mechanic can be added without changing `Core`.

The example mechanic is `Lancer`, a classic unit archetype that can strike an attackable target exactly two cells away, fall back to adjacent melee, and move if no attack rule can progress.

## Changed Files

- `src/Features/UnitsClassic/ClassicActionRuleRecipes.hpp`
- `src/Features/UnitsClassic/ClassicActionRuleRecipes.cpp`
- `src/Features/UnitsClassic/ClassicUnitArchetypes.cpp`
- `tests/ScenarioBehaviorTests.cpp`
- `tests/EcsActionRuleTests.cpp`
- `examples/add-new-mechanic.md`

No `Core`, `App`, `IO`, CMake, or scenario command file changes are required.

## Pattern

1. Compose behavior from existing rule handlers in `ClassicActionRuleRecipes`.
2. Register the archetype in `ClassicUnitArchetypes`.
3. Exercise it through the existing generic `SPAWN` command.
4. Add focused tests for both the rule recipe and scenario behavior.

The `Lancer` reach rule uses existing handlers:

```text
condition: core.always
selector: battle.effective-range
picker: core.random-one
effect: battle.damage
amount: battle.strength
distance: exactly 2
```

That keeps the extension inside feature-owned code while reusing the generic action-rule pipeline.

## Deterministic Check

```bash
./build/battle_sim commands_example.txt
ctest --test-dir build --output-on-failure
```

For a data-driven version of the same idea, see `config/archetypes/spearman.json`.
