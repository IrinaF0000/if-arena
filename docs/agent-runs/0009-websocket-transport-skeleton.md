# 0009 WebSocket Transport Skeleton

Date: 2026-05-19

## Changed files

- `src/battle_transport_ws/WebSocketSession.hpp` - added WebSocket session adapter skeleton and message limits.
- `src/battle_transport_ws/placeholder.cpp` - implemented inbound size check, shared protocol parsing, outbound bound, and close state.
- `tests/unit/transport/WebSocketSessionTests.cpp` - added oversized, malformed, valid parse, and outbound bound tests.
- `src/battle_transport_ws/README.md` - documented current skeleton boundary.
- `docs/architecture/TRANSPORT_ABSTRACTION.md` - documented current WebSocket skeleton behavior.
- `docs/telegram/TELEGRAM_SECURITY.md` - documented current transport/auth boundary.

## Checks run

- `cmake --build cmake-build-0004 --parallel` - passed.
- Manual unit compile: `g++ -std=c++20 -Wall -Wextra -Wpedantic -I src/battle_transport_ws -I src/battle_protocol -I src/battle_backend src/battle_transport_ws/placeholder.cpp src/battle_protocol/placeholder.cpp tests/unit/transport/WebSocketSessionTests.cpp -o cmake-build-0004/WebSocketSessionTests.exe` - passed.
- `cmake-build-0004/WebSocketSessionTests.exe` - passed.
- `cmake-build-0004/TcpFrameCodecTests.exe` - passed.
- `cmake-build-0004/SessionRegistryTests.exe` - passed.
- `cmake-build-0004/ProtocolEnvelopeTests.exe` - passed.
- `cmake-build-0004/BattleEngineFacadeTests.exe` - passed.
- `ctest --test-dir cmake-build-0004 --output-on-failure` - passed; no tests are registered in root CMake yet.
- `python scripts/ci/scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed with Python shim in Git Bash.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed.

## Risks and follow-up

- No concrete WebSocket library is selected yet.
- This packet does not implement async I/O, WSS deployment, Telegram auth validation, rate limiting, or real session handshakes.
- Architecture-Agent, Security-Agent, Performance-Agent, and Verification-Agent reviews are required.
