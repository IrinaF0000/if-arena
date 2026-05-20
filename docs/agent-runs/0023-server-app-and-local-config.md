# Task 0023: Server App And Local Config

## Status

Implemented on branch `agent/0023-server-app-and-local-config`.

## Changed files

- `src/battle_server_app/main.cpp`
- `src/battle_server_app/README.md`
- `config/examples/server.local.json`
- `config/examples/server.public.json`
- `config/examples/README.md`
- `scripts/run_local_server.sh`
- `scripts/run_public_server.sh`
- `docs/deployment/DEPLOYMENT.md`
- `README.md`
- `docs/agent-runs/0023-server-app-and-local-config.md`

## Implementation notes

- Added `--config`, `--local`, `--check-config`, and `--help`.
- Added config loading and validation for server timing, backend capacity, transport intent, security limits, and metrics cadence.
- Initialized `SessionRegistry` and `MatchManager` with `BackendLimits` derived from config.
- Added safe startup/backend metrics logging without dumping config contents, Telegram auth payloads, or secret values.
- Added actionable not-implemented startup failures for enabled TCP/WebSocket listeners.
- Updated run scripts to pass config paths instead of unused host/port flags.

## Test matrix

- Local config validates with `--check-config`.
- Public config validates with `--check-config` and enforces public safety requirements.
- Local server startup with enabled transports exits non-zero with clear TCP/WebSocket not-implemented errors instead of silently succeeding.

## Checks run

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `build/battle_server_app.exe --config config/examples/server.local.json --check-config`
- `build/battle_server_app.exe --config config/examples/server.public.json --check-config`
- Expected startup failure smoke: `build/battle_server_app.exe --config config/examples/server.local.json` exits non-zero with TCP/WebSocket not-implemented errors.
- `python -m json.tool config/examples/server.local.json`
- `python -m json.tool config/examples/server.public.json`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh` via Git Bash with the bundled Python shim
- `git diff --check`

## Risks and follow-up

- The config parser is intentionally scoped to the server config shape. A full JSON library can replace it when dependency policy allows.
- Real TCP/WebSocket listener startup remains for later transport integration packets.
