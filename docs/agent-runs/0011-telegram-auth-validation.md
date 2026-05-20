# 0011 Telegram Auth Validation

Date: 2026-05-19

## Changed files

- `src/battle_backend/security/TelegramAuth.hpp` - added Telegram auth validator API.
- `src/battle_backend/placeholder.cpp` - implemented initData parsing, HMAC-SHA256 validation, freshness checks, and redaction helper.
- `tests/security/TelegramAuthTests.cpp` - added valid fixture, invalid hash, stale auth date, oversized init data, and redaction tests.
- `src/battle_backend/security/README.md` - documented current helpers.
- `docs/telegram/TELEGRAM_SECURITY.md` - documented backend validator behavior and replay follow-up.
- `docs/security/SECURITY_REQUIREMENTS.md` - documented current implementation note.

## Checks run

- `cmake --build cmake-build-0004 --parallel` - passed.
- Manual unit compile: `g++ -std=c++20 -Wall -Wextra -Wpedantic -I src/battle_backend src/battle_backend/placeholder.cpp tests/security/TelegramAuthTests.cpp -o cmake-build-0004/TelegramAuthTests.exe` - passed.
- `cmake-build-0004/TelegramAuthTests.exe` - passed.
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

- Replay/duplicate initData protection is not implemented yet.
- Bot token loading from real secure runtime config is not wired in this packet.
- Security-Agent, Architecture-Agent, and Verification-Agent reviews are required.
