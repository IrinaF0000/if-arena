# 0008 Qt Client Network Skeleton

Date: 2026-05-19

## Changed files

- `src/battle_qt_client/network/NetworkClient.hpp` - added connection state, endpoint, and intent-send skeleton.
- `src/battle_qt_client/placeholder.cpp` - included the skeleton in the placeholder target.
- `src/battle_qt_client/network/README.md` - documented separation and signal/slot plan.
- `src/battle_qt_client/README.md` - documented current network skeleton.
- `tests/manual/qt_client_checklist.md` - added connect/disconnect/reconnect checklist items.

## Checks run

- `cmake -S . -B cmake-build-0008-qt -G Ninja -DBATTLE_BUILD_QT_CLIENT=ON -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build cmake-build-0008-qt --parallel` - passed.
- `ctest --test-dir cmake-build-0008-qt --output-on-failure` - passed; no tests are registered in root CMake yet.
- `cmake-build-0004/TcpFrameCodecTests.exe` - passed.
- `cmake-build-0004/SessionRegistryTests.exe` - passed.
- `cmake-build-0004/ProtocolEnvelopeTests.exe` - passed.
- `cmake-build-0004/BattleEngineFacadeTests.exe` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed with Python shim in Git Bash.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed.

## Risks and follow-up

- Real `QObject`, `QTcpSocket`, protocol parsing, and widget integration remain for later Qt packets.
- Qt-Agent, Architecture-Agent, and Verification-Agent reviews are required.
