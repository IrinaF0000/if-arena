# Core Extraction Checklist

## Before extraction

- [x] Old project copied into `external/battle_simulation_snapshot/`.
- [x] Build artifacts removed from snapshot.
- [x] Current old project behavior understood at audit level; see `docs/agent-notes/0003-core-extraction-audit.md`.
- [x] Reference scenario selected: old `tests/DeterministicScenarioTests.cpp::canonicalLegacyScenarioMatchesGoldenEventLog`.
- [x] Expected reference output/replay identified: golden legacy event log in `tests/DeterministicScenarioTests.cpp` and sample JSON replay at `tools/replay-viewer/traces/basic-battle.json`.

## Extract modules

- [x] World/state model.
- [x] Unit/entity/component model.
- [ ] Action/rule execution.
- [x] Tick/update loop.
- [ ] Scenario loading.
- [ ] Feature pack/archetype loading.
- [x] Event generation.
- [ ] Replay/trace support, if reusable.

## Do not put into `battle_core`

- [ ] Old `main.cpp` application flow.
- [ ] TCP server code.
- [ ] Qt client code.
- [ ] CLI UI code.
- [ ] Deployment scripts.
- [ ] Load client code.
- [ ] HTML/GIF generation unless isolated as pure replay output utility.

## Add public facade

- [x] `BattleEngine` or equivalent.
- [x] `MatchConfig`.
- [x] `PlayerCommand`.
- [x] `CommandResult`.
- [x] `BattleSnapshot`.
- [x] `BattleEvent`.
- [x] `tick()` or fixed-step update method.

## Add arena support

- [x] Player ownership.
- [ ] Hero archetype.
- [ ] Objective entity.
- [ ] Hazard entities.
- [ ] Move/attack/interact validation.
- [x] Match result state.

## Acceptance

- [x] `battle_core` builds as static library.
- [ ] Reference scenario deterministic test passes.
- [x] Local in-process arena match test passes.
- [x] No TCP/Qt dependencies in `battle_core`.
- [x] Documentation updated.
