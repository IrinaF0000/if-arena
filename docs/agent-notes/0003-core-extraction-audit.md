# 0003 Core Extraction Audit

Date: 2026-05-19

## Scope

Audited `external/battle_simulation_snapshot/battle_sim/` as read-only source material for future `src/battle_core/` extraction. No production source was changed.

## Old Runtime Shape

The old CMake target `battle_sim_lib` combines:

- `src/Core/` - generic ECS/runtime infrastructure.
- `src/Features/Battle/` - battle state, systems, mutations, selectors, policies, events, and battle facade.
- `src/Features/UnitsClassic/` - built-in unit archetypes and action recipes.
- `src/App/` - scenario runner, config loading, feature-pack wiring, trace inspection.
- `src/IO/` - legacy text commands, legacy output adapter, JSON trace writer.

Runtime flow is text commands -> app scenario runner -> IO command handlers -> `BattleSimulationFacade` -> `EngineRunner`/systems -> battle events -> legacy output or JSON trace.

## Extraction Map

| Old path | Classification | Future target |
| --- | --- | --- |
| `src/Core/Ecs/` | core | `src/battle_core/` internal ECS/world model, if retained. |
| `src/Core/Engine/` | core | `src/battle_core/` tick/update runtime, scheduler, resources, mutations. |
| `src/Core/Registry/` | core | `src/battle_core/` handler/feature registry internals, simplified if possible. |
| `src/Core/Config/` | core candidate | `src/battle_core/` config structs only if needed by match setup. |
| `src/Features/Battle/Components/` | core | Battle entity state/components. |
| `src/Features/Battle/Resources/` | core | Map/occupancy and spatial state. |
| `src/Features/Battle/Systems/` | core | Tick systems for turn, spawn, movement, march. |
| `src/Features/Battle/Mutations/` | core | Authoritative state mutation helpers. |
| `src/Features/Battle/Selectors/`, `Policies/`, `Conditions/`, `Effects/`, `Values/` | core | Rule helpers for actions and target resolution. |
| `src/Features/Battle/Events/` | core/API candidate | Convert to stable `BattleEvent` DTOs for backend fanout/replay. |
| `src/Features/Battle/BattleSimulationFacade.*` | core/API seed | Use as input to future `BattleEngine` facade, but replace scenario-specific commands with arena commands. |
| `src/Features/UnitsClassic/` | core candidate | Reference archetype/recipe source; adapt only reusable unit stats/actions. |
| `config/` and `config/archetypes/` | core candidate | Match/archetype config seed; validate before runtime use. |
| `tests/Ecs*Tests.cpp` | test reference | Unit tests for extracted ECS/action behavior. |
| `tests/ScenarioBehaviorTests.cpp` | test reference | Behavioral regression cases for extracted core. |
| `tests/DeterministicScenarioTests.cpp` | reference scenario | Golden deterministic behavior baseline. |
| `src/App/` | app | Keep out of `battle_core`; useful as reference for composition and scenario execution only. |
| `src/IO/LegacyCommands/` | app/compat | Keep out of `battle_core`; text command parser and legacy output are not arena core APIs. |
| `src/IO/Trace/JsonTraceWriter.*` | tool/app candidate | Keep out of initial `battle_core`; reusable only as optional replay/export adapter. |
| `src/main.cpp` | app | Do not extract into `battle_core`. |
| `tools/replay-viewer/`, `docs/replay-viewer/` | tools | Browser replay reference only; no core dependency. |
| `tools/local-runner/` | tools | Local demo helper only. |
| `.github/`, old CI docs, old agent seeds | ignore | Do not extract. |

## Reference Scenario

Primary deterministic baseline:

- File: `tests/DeterministicScenarioTests.cpp`
- Case: `canonicalLegacyScenarioMatchesGoldenEventLog`
- Behavior: fixed small map, two swordsmen, deterministic tick/event order, legacy `MAP_CREATED`, `UNIT_SPAWNED`, `UNIT_ATTACKED`, and `UNIT_DIED` output.

Replay/reference output:

- Golden event log embedded in `tests/DeterministicScenarioTests.cpp`.
- JSON sample trace: `tools/replay-viewer/traces/basic-battle.json`.

## Risks And Unclear Dependencies

- The old `Core` is intentionally generic; extraction should avoid over-preserving extension machinery if the arena MVP needs a smaller facade.
- Old battle rules are turn/scenario oriented, while IF Arena needs authoritative multi-player match commands and snapshots.
- Current map/resource rules use rectangular occupancy and 8-neighbor movement; future arena map fairness and objective rules must be reconciled before facade work.
- Legacy command parsing, text output, JSON trace, and browser replay are useful references but must not become `battle_core` dependencies.
- Config/archetype files are untrusted input in the new project and need validation before reuse.

## Checks Run

- `python scripts/ci/scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed with Python shim in Git Bash.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed.
- CMake build not run; task is documentation-only audit with no production source changes.

## Review

- Architecture-Agent review required before facade extraction work.
- Verification-Agent review required.
