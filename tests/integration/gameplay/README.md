# Gameplay integration scenarios

Scenario JSON under `tests/scenarios/` owns routes, commands, ticks, and expectations.

`scenario_runner.py` is transport-generic. Desktop wrappers run the scenario over raw TCP; mobile wrappers run the same scenario over WebSocket. Test code may choose the scenario file path and transport only.
