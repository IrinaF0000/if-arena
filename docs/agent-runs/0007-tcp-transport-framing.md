# 0007 TCP Transport Framing

Date: 2026-05-19

## Changed files

- `src/battle_transport_tcp/TcpFrameCodec.hpp` - added standalone TCP length-prefix frame encoder/decoder.
- `src/battle_transport_tcp/placeholder.cpp` - implemented codec behavior and explicit error state.
- `tests/unit/transport/TcpFrameCodecTests.cpp` - added partial, combined, oversized, invalid length, and encoder-bound tests.
- `src/battle_transport_tcp/README.md` - documented codec behavior.
- `docs/architecture/PROTOCOL.md` - documented current TCP codec helper.

## Checks run

- `cmake --build cmake-build-0004 --parallel` - passed.
- Manual unit compile: `g++ -std=c++20 -Wall -Wextra -Wpedantic -I src/battle_transport_tcp src/battle_transport_tcp/placeholder.cpp tests/unit/transport/TcpFrameCodecTests.cpp -o cmake-build-0004/TcpFrameCodecTests.exe` - passed.
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

- Root CMake does not yet register `tests/unit` executables, so TCP codec tests are compiled and run manually.
- This packet does not implement sockets, async I/O, handshakes, idle timeout, slow-reader policy, or protocol JSON validation in the transport.
- Security-Agent, Performance-Agent, and Verification-Agent reviews are required.
