# Game Loop

This document describes the current CLI scenario runtime and the reusable scheduled engine path.

## CLI Scenario Path

`battle_sim <commands-file>` opens one command file and passes it to `battle_sim::app::runScenario`.

The runner wires:

- `EventLog` over the output stream;
- `LegacyEventAdapter` as a type-erased event sink;
- `BattleSimulationFacade` over a `core::engine::GameContext`;
- `Features/Battle` handlers and systems;
- `EntityArchetypeRegistry` with classic unit archetypes;
- `IO/LegacyCommands::CommandParser` with legacy map, spawn, and march commands.

Command parsing is a setup phase. Each command is handled immediately:

- `CREATE_MAP` creates a `BattleMapResource` and emits `MAP_CREATED` at tick `1`.
- `SPAWN_*` and generic `SPAWN` create battle entity recipes, place blocking entities on the battle map, attach feature-owned ECS components, and emit `UNIT_SPAWNED` at tick `1`.
- `MARCH` validates the target, stores a feature-owned march target component, and emits `MARCH_STARTED` at tick `1`.

After the file is parsed, `BattleSimulationFacade::run` runs `core::engine::EngineRunner` over the systems registered by the active feature packs.

## Battle Turn Loop

The battle feature pack registers scheduled systems that start gameplay ticks at `2`. The scheduler stops immediately if no battle map resource exists.

Each tick:

1. Iterate entities in creation order.
2. Skip entities that cannot act this turn.
3. Execute action rules through feature-owned handlers and registries.
4. Clean up dead entities after all eligible entities had their turn.
5. Stop if no actions executed in the tick.
6. Advance the world tick and repeat while more than one active entity remains.

Action execution sorts rules by descending priority with stable ordering for ties. Each rule follows:

```text
condition -> selector -> target picker -> primary effect -> after effect
```

The action budget is reduced by each successful rule cost. The entity stops acting when budget runs out, no rule can progress, or the entity is removed.

## Scheduled Engine Path

The generic engine path is represented by `core::engine::EngineRunner` and `Scheduler`. It is the CLI battle runtime path.

The scheduled loop runs:

```text
Startup once
while not stopped and below maxTicks:
  BeforeTick
  Tick
  AfterTick
  Cleanup
  advance world tick
Shutdown once
```

Systems within a phase run in registration order. A scheduled system can request a stop by returning `true`; `EngineRunner` stops between phases.

## Stable IO Surface

Legacy command syntax and legacy event text are stable IO surfaces. Game-loop changes preserve command files, `UNIT_*` output, and deterministic replay behavior unless a task explicitly updates those expectations.
