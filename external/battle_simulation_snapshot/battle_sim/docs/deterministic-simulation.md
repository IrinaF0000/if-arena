# Deterministic Simulation

Deterministic validation treats a scenario as an ordered text-command input and the legacy event log as the compatibility output.

## Inputs

- Scenario commands are consumed in file order.
- A fixed RNG seed must be supplied by the app or test harness when a scenario can choose between equivalent targets or movement cells.
- Command syntax stays compatible with existing legacy scenario files.

The current CLI keeps the historical command-file interface. Tests use `battle_sim::app::RunOptions::rngSeed` and config-level `rngSeed` to make replay checks reproducible.

## Outputs

- The legacy `UNIT_*`, `MAP_CREATED`, and `MARCH_*` lines are the golden-output surface.
- Output order is significant.
- Validation diagnostics on stderr are part of deterministic replay checks when invalid commands are exercised.

## Validation

`deterministic_scenario_tests` covers two cases:

- a canonical legacy-command scenario compared with an exact golden event log;
- repeated runs of one fixed-seed scenario compared against each other.

The baseline verification commands run this target alongside the existing behavior, ECS world, action-rule, and action-executor tests.
