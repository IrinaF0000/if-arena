# Task 0025: TCP Vertical Slice Integration

## Status

Implemented on branch `agent/0025-tcp-vertical-slice-integration`.

Pipeline state: `commit-ready`.

## Changed files

- `CMakeLists.txt`
- `src/battle_transport_tcp/TcpFrameCodec.hpp`
- `src/battle_transport_tcp/TcpFrameCodec.cpp`
- `src/battle_server_app/main.cpp`
- `src/battle_backend/BackendFoundation.cpp`
- `src/battle_cli_client/CliClientMain.cpp`
- `src/battle_transport_tcp/README.md`
- `src/battle_server_app/README.md`
- `src/battle_cli_client/README.md`
- `docs/architecture/TRANSPORT_ABSTRACTION.md`
- `docs/architecture/PROTOCOL.md`
- `README.md`
- `tests/integration/server/README.md`
- `tests/integration/server/cli_idle.script`
- `tests/integration/server/tcp_vertical_slice_smoke.py`
- `docs/agent-runs/0025-tcp-vertical-slice-integration.md`

## Implementation notes

- Added portable local TCP `TcpListener`/`TcpConnection` wrappers around the existing length-prefixed frame codec.
- Wired `battle_server_app` TCP accept/read/write to backend session creation, auth, match create/join, input command submission, disconnect, and outbound snapshot/event flushing.
- Added handshake timeout, idle ping/timeout handling, malformed-envelope rejection, oversized frame fail-close behavior, and backend outbound queue backpressure.
- Updated the CLI to use real raw TCP by default while keeping explicit `--fake-connect` transcript mode.
- Serialized backend snapshots/event batches as protocol envelopes so TCP clients receive transport-independent message types.

## Test Impact Matrix

Changed behavior:
- Raw TCP server accepts local clients and maps connections to backend sessions.
- CLI opens real TCP by default and can run create/join scripts against the server.
- Hostile TCP frames/messages fail closed or return structured protocol errors.
- Backend snapshot/event payloads are sent as protocol envelopes.

Tests added/updated:
- Happy path: `tests/integration/server/tcp_vertical_slice_smoke.py` authenticates two TCP clients, creates/joins a match, receives snapshots, and submits an accepted command.
- Corner cases: frame codec unit tests already cover partial and combined frames; integration smoke covers handshake timeout.
- Invalid input / hostile input: integration smoke covers malformed JSON, unknown type, and oversized frame length.
- Authority / ownership: existing backend tests cover wrong session/player ownership and duplicate sequence rejection; TCP path does not accept claimed player/team/state fields.
- Resource bounds / performance: existing backend tests cover bounded command/outbound queues and slow-session closure; integration smoke covers pre-allocation oversized frame rejection and handshake timeout.
- Regression: `ctest --test-dir build --output-on-failure` covers core, protocol, backend, TCP frame codec, WS skeleton, and Telegram auth tests.
- Manual UI checks: none; no UI changed.

Not tested and why:
- Real slow-reader socket-buffer saturation is not stress-tested here; backend queue overflow and send timeout behavior are covered by unit tests/configured bounds, while full load testing is reserved for task 0028.
- Public deployment/TLS behavior is not tested; raw TCP slice is local-only.

## Checks run

- `git branch --show-current`
- `git status`
- `git log --oneline -5`
- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/integration/server/tcp_vertical_slice_smoke.py`
- Manual two-CLI TCP smoke with `battle_server_app --max-clients 2`, one creator using `cli_idle.script`, and one joiner using `cli_scenario_b.script`.
- `git diff --check`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `bash scripts/ci/validate_structure.sh` via Git Bash with a temporary `python3` function pointing to bundled Python.

## Required reviews

- Architecture-Agent: approve.
  - TCP/socket code remains outside `battle_core`.
  - Backend remains authoritative; clients send protocol intentions only.
  - `battle_server_app` is still a composition root, though this local slice keeps blocking per-connection handling there until the async transport hardening task.
- Security-Agent: approve-with-minors, fixed before commit-ready.
  - Fixed: server error payloads now JSON-escape code/message strings.
  - Oversized frames, malformed JSON, unknown message type, and handshake timeout are covered by integration smoke.
  - Secret scan passed.
- Performance-Agent: approve-with-minors.
  - Bounds exist for frame size, command queues, outbound messages/bytes, send timeout, handshake timeout, and idle timeout.
  - Remaining risk is accepted for this local slice: blocking per-connection threads and no socket-buffer slow-reader stress test until task 0028.
- Verification-Agent: approve.
  - Required checks above passed.
  - Test Impact Matrix is present.
  - No forbidden directories changed.

## Known risks

- `CMakeLists.txt` is changed to link Winsock on Windows, but the task packet did not list it in allowed files. The change is required for the new Windows socket usage to link.
- The TCP server loop is a local blocking-thread slice, not the final async production transport.
- WebSocket remains intentionally skipped until task 0026.
