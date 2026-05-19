# System Order

This document lists the current CLI scenario order and the reusable scheduler phase order.

## CLI Scenario Order

The command-file runtime uses `App::ScenarioRunner`, `IO/LegacyCommands`, and `Features/Battle::BattleSimulationFacade`.

Setup at tick `1`:

| Order | System or adapter | Responsibility |
| --- | --- | --- |
| 1 | `IO/LegacyCommands::CommandParser` | Reads non-empty, non-comment command lines in file order. |
| 2 | `CREATE_MAP` handler | Creates `BattleMapResource` and emits `MAP_CREATED`. |
| 3 | `SPAWN_*` / `SPAWN` handlers | Create battle recipes from classic archetypes and command stats. |
| 4 | `BattleSpawnSystem` | Validates spawn, creates entity, writes feature-owned components, updates map occupancy, emits `UNIT_SPAWNED`. |
| 5 | `MARCH` handler | Starts movement intent through `BattleMarchSystem`, emits `MARCH_STARTED`, and stores target state. |

Gameplay ticks run through `EngineRunner` and start at tick `2`:

| Order | System | Responsibility |
| --- | --- | --- |
| 1 | `Startup` phase | Initializes the gameplay tick to `2` when a battle map exists. |
| 2 | `BeforeTick` phase | Stops if the battle cannot continue. |
| 3 | `Tick` phase / `BattleTurnSystem` | Iterates active entities in creation order. |
| 4 | `canActThisTurn` policy | Applies battle lifecycle rules. |
| 5 | Rule budget loop | Executes rules by priority and action budget. |
| 6 | Rule condition | Checks whether the rule may run. |
| 7 | Rule selector | Builds candidate targets. |
| 8 | Target picker | Chooses no target, first target, random target, or all targets. |
| 9 | Primary effect | Applies damage, healing, movement, state change, self-destruction, or no effect. |
| 10 | After effect | Applies configured follow-up state or destruction behavior. |
| 11 | `AfterTick` phase / battle cleanup | Removes dead entities and stops if the tick executed no actions. |

The loop stops when one or fewer active entities remain, or when a tick executes no actions.

## Scheduler Phase Order

`core::engine::Scheduler` owns the neutral phase order:

1. `Startup`
2. `BeforeTick`
3. `Tick`
4. `AfterTick`
5. `Cleanup`
6. `Shutdown`

Within each phase, systems run in the order they were added.

`EngineRunner` executes `Startup` once, then repeats `BeforeTick`, `Tick`, `AfterTick`, and `Cleanup` until stopped or `GameSettings::maxTicks` is reached. It advances the world tick after `Cleanup`, then runs `Shutdown` once.

## Feature-Pack Registration Order

The built-in feature-pack catalog registers these pack IDs:

| Order in catalog | Feature pack | Current responsibility |
| --- | --- | --- |
| 1 | `battle.basic` | Battle policies, value resolvers, condition handlers, target selectors, and effects. |
| 2 | `units.classic` | Classic unit archetypes; requires `battle.basic`. |
| 3 | `io.legacy` | Legacy text-command IO shell. |

The catalog stores factories. Runtime feature activation order is controlled by the caller/config path that selects packs; `units.classic` expects `battle.basic` to be active first.
