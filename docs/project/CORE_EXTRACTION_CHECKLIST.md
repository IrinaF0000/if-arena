# Core Extraction Checklist

## Before extraction

- [ ] Old project copied into `external/battle_simulation_snapshot/`.
- [ ] Build artifacts removed from snapshot.
- [ ] Current old project behavior understood.
- [ ] Reference scenario selected.
- [ ] Expected reference output/replay identified.

## Extract modules

- [ ] World/state model.
- [ ] Unit/entity/component model.
- [ ] Action/rule execution.
- [ ] Tick/update loop.
- [ ] Scenario loading.
- [ ] Feature pack/archetype loading.
- [ ] Event generation.
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

- [ ] `BattleEngine` or equivalent.
- [ ] `MatchConfig`.
- [ ] `PlayerCommand`.
- [ ] `CommandResult`.
- [ ] `BattleSnapshot`.
- [ ] `BattleEvent`.
- [ ] `tick()` or fixed-step update method.

## Add arena support

- [ ] Player ownership.
- [ ] Hero archetype.
- [ ] Objective entity.
- [ ] Hazard entities.
- [ ] Move/attack/interact validation.
- [ ] Match result state.

## Acceptance

- [ ] `battle_core` builds as static library.
- [ ] Reference scenario deterministic test passes.
- [ ] Local in-process arena match test passes.
- [ ] No TCP/Qt dependencies in `battle_core`.
- [ ] Documentation updated.
