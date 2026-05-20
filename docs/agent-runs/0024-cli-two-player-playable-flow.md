# Task 0024: CLI Two-Player Playable Flow

## Status

Implemented on branch `agent/0024-cli-two-player-playable-flow`.

## Changed files

- `src/battle_cli_client/CliClientMain.cpp`
- `src/battle_cli_client/README.md`
- `tests/integration/server/cli_scenario_b.script`
- `README.md`
- `docs/agent-runs/0024-cli-two-player-playable-flow.md`

## Implementation notes

- Added CLI args for endpoint, demo auth metadata, create/join flow, match id, script file, and fake-connect mode.
- Added scripted command parsing for intent-only commands: `move`, `aim`, `attack`, `dash`, `interact`, and `stop`.
- Generated protocol envelopes through `battle_protocol::serializeEnvelope` and validated client envelopes before printing.
- Fake-connect mode is explicitly labeled and does not open TCP or mutate client-owned game state.
- Added a Scenario B script matching the canonical objective route used by backend fake-session tests.

## Checks run

- `cmake --build build --target battle_cli_client --parallel`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `build/battle_cli_client.exe --fake-connect --create --match-id local-match`
- `build/battle_cli_client.exe --fake-connect --create --match-id local-match --script tests/integration/server/cli_scenario_b.script`
- `build/battle_cli_client.exe --fake-connect --join LOCAL1 --match-id local-match --display-name cli-two`
- Expected interactive failure smoke: `build/battle_cli_client.exe --interactive`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh` via Git Bash with the bundled Python shim
- `git diff --check`

## Risks and follow-up

- Real TCP mode is intentionally deferred to task 0025 because task 0024 does not allow CMake target rewiring to backend/transport libraries.
- Fake snapshots/events are transcript labels only; real authoritative snapshots arrive with the TCP vertical slice.
