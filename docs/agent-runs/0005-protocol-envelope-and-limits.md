# 0005 Protocol Envelope And Limits

Date: 2026-05-19

## Changed files

- `src/battle_protocol/Protocol.hpp` - added envelope DTOs, message types, limits, parse/serialize result types, and error codes.
- `src/battle_protocol/placeholder.cpp` - implemented bounded top-level envelope parser and serializer.
- `tests/unit/protocol/ProtocolEnvelopeTests.cpp` - added valid and negative protocol envelope tests.
- `src/battle_protocol/README.md` - documented public helpers.
- `docs/architecture/PROTOCOL.md` - documented recognized types, parser behavior, and C++ limits.

## Checks run

- `cmake -S . -B cmake-build-0004 -G Ninja -DBATTLE_BUILD_QT_CLIENT=OFF -DBATTLE_BUILD_TESTS=ON` - reused from 0004; passed.
- `cmake --build cmake-build-0004 --parallel` - passed.
- Manual unit compile: `g++ -std=c++20 -Wall -Wextra -Wpedantic -I src/battle_protocol src/battle_protocol/placeholder.cpp tests/unit/protocol/ProtocolEnvelopeTests.cpp -o cmake-build-0004/ProtocolEnvelopeTests.exe` - passed.
- `cmake-build-0004/ProtocolEnvelopeTests.exe` - passed.
- `ctest --test-dir cmake-build-0004 --output-on-failure` - passed; no tests are registered in root CMake yet.
- `python scripts/ci/scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed with Python shim in Git Bash.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed.
- C++ dependency text check over `src/battle_protocol/*.cpp` and `*.hpp` - no transport/client dependency terms found.

## Risks and follow-up

- Root CMake does not yet register `tests/unit` executables, so protocol tests are compiled and run manually.
- Payload-specific schema validation is intentionally deferred to later protocol/backend packets.
- Security-Agent, Architecture-Agent, and Verification-Agent reviews are required.
