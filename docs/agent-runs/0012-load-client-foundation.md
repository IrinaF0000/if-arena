# 0012 Load Client Foundation

Date: 2026-05-19

## Changed files

- `tools/load_client/placeholder.cpp` - added bounded dry-run config parsing and markdown report writer.
- `tools/load_client/README.md` - documented current dry-run usage.
- `docs/performance/PERFORMANCE_AND_LOAD_TESTING.md` - documented load client foundation state.
- `reports/load/README.md` - added report directory note.
- `reports/load/0012-dry-run.md` - recorded sample dry-run output.

## Checks run

- `cmake --build cmake-build-0004 --parallel` - passed.
- `cmake-build-0004\battle_load_client.exe --dry-run --endpoint 127.0.0.1:4000 --clients 5 --duration 3 --command-rate 2 --scenario connect_only --output reports/load/0012-dry-run.md` - passed; wrote report.
- `cmake-build-0004\BattleEngineFacadeTests.exe` - passed.
- `cmake-build-0004\ProtocolEnvelopeTests.exe` - passed.
- `cmake-build-0004\SessionRegistryTests.exe` - passed.
- `cmake-build-0004\TcpFrameCodecTests.exe` - passed.
- `cmake-build-0004\WebSocketSessionTests.exe` - passed.
- `cmake-build-0004\TelegramAuthTests.exe` - passed.
- `ctest --test-dir cmake-build-0004 --output-on-failure` - passed; no CTest tests are registered in the current root CMake configuration.
- `python scripts\ci\scan_secrets.py` - passed.
- `python scripts\agent\validate_agent_harness.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed using Git Bash with a local `python3` shim.
- `git diff --check` - passed; Git reported CRLF normalization warnings only.

## Risks and follow-up

- Live TCP/WebSocket simulated clients, auth/join loop, latency sampling, and slow/spam/invalid scenarios remain future work.
- Performance-Agent and Verification-Agent reviews are required.
- The next coordination-plan packet, `0013-ci-cd-guardrails`, is CI/CD-only and should be handled in an isolated branch/worktree rather than mixed into this broad implementation diff.
