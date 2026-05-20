# Foundation Through 0015 Split Plan

Date: 2026-05-20

## Source State

- Safety branch: `agent/foundation-through-0015-integration`.
- Safety snapshot commit: `9398278` (`Safety snapshot foundation through 0015`).
- Base branch for semantic slices: `master` at `20270a8`.
- Snapshot size: 66 changed paths, 4202 insertions, 57 deletions.
- Ignored build/run directories were not included in the safety snapshot.

The safety snapshot is a preservation commit only. It must not be merged as a single release commit.

## Proposed Semantic Branches

### 1. `agent/core-protocol-foundation`

Scope:

- Task packets `0002`, `0003`, `0004`, `0005`.
- Original snapshot import notes.
- Battle core facade.
- Protocol envelope, limits, parser/serializer, and protocol tests.

Files:

- `external/battle_simulation_snapshot/README.md`
- `docs/project/CORE_EXTRACTION_CHECKLIST.md`
- `docs/project/ORIGINAL_PROJECT_IMPORT.md`
- `docs/agent-notes/0003-core-extraction-audit.md`
- `docs/agent-runs/0001a-sanitize-snapshot.md`
- `docs/agent-runs/0002-import-old-project-snapshot.md`
- `docs/agent-runs/0004-battle-core-facade.md`
- `docs/agent-runs/0005-protocol-envelope-and-limits.md`
- `docs/architecture/PROTOCOL.md`
- `docs/architecture/TECHNICAL_ARCHITECTURE.md`
- `src/battle_core/**`
- `src/battle_protocol/**`
- `tests/unit/core/BattleEngineFacadeTests.cpp`
- `tests/unit/protocol/ProtocolEnvelopeTests.cpp`
- core/protocol portions of `CMakeLists.txt`

Checks:

- `cmake -S . -B cmake-build-split -G Ninja -DBATTLE_BUILD_QT_CLIENT=OFF`
- `cmake --build cmake-build-split --parallel`
- `ctest --test-dir cmake-build-split --output-on-failure`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

### 2. `agent/backend-session-foundation`

Scope:

- Task packet `0006`.
- Backend session registry, outbound queue bounds, and backend tests.

Files:

- `src/battle_backend/README.md`
- `src/battle_backend/Session.hpp`
- `src/battle_backend/placeholder.cpp` session-only sections
- `tests/unit/backend/SessionRegistryTests.cpp`
- `docs/agent-runs/0006-backend-session-model.md`
- `docs/architecture/TRANSPORT_ABSTRACTION.md` backend/session sections
- backend session portions of `CMakeLists.txt`

Checks:

- Baseline CMake configure/build/ctest set
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

### 3. `agent/transports-foundation`

Scope:

- Task packets `0007` and `0009`.
- TCP frame codec.
- WebSocket protocol adapter.

Files:

- `src/battle_transport_tcp/**`
- `src/battle_transport_ws/**`
- `tests/unit/transport/TcpFrameCodecTests.cpp`
- `tests/unit/transport/WebSocketSessionTests.cpp`
- `docs/agent-runs/0007-tcp-transport-framing.md`
- `docs/agent-runs/0009-websocket-transport-skeleton.md`
- transport sections of `docs/architecture/PROTOCOL.md`
- transport sections of `docs/architecture/TRANSPORT_ABSTRACTION.md`
- WebSocket transport notes in `docs/telegram/TELEGRAM_SECURITY.md`
- transport portions of `CMakeLists.txt`

Checks:

- Baseline CMake configure/build/ctest set
- TCP negative tests for partial frames, combined frames, oversize frames, invalid length, and buffer bounds
- WebSocket tests for malformed, oversized, and closed-session messages
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

### 4. `agent/clients-foundation`

Scope:

- Task packets `0008` and `0010`.
- Qt network skeleton.
- Telegram Mini App protocol validation and touch-control wiring.

Files:

- `src/battle_qt_client/**`
- `tests/manual/qt_client_checklist.md`
- `frontend/telegram_mini_app/README.md`
- `frontend/telegram_mini_app/src/main.ts`
- `frontend/telegram_mini_app/src/network/WebSocketClient.ts`
- `frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts`
- `frontend/telegram_mini_app/src/ui/TouchControls.ts`
- `docs/agent-runs/0008-qt-client-network-skeleton.md`
- `docs/agent-runs/0010-telegram-mini-app-skeleton.md`
- `docs/telegram/TELEGRAM_MINI_APP.md`
- frontend/client sections of `docs/architecture/PROTOCOL.md`

Checks:

- Baseline CMake configure/build/ctest set with `-DBATTLE_BUILD_QT_CLIENT=OFF`
- `npm run typecheck`, `npm run lint`, and `npm run build` in `frontend/telegram_mini_app` if npm is available
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

### 5. `agent/load-observability-foundation`

Scope:

- Task packet `0012`.
- Dry-run load client and load reporting docs.

Files:

- `tools/load_client/README.md`
- `tools/load_client/placeholder.cpp`
- `reports/load/README.md`
- `reports/load/0012-dry-run.md`
- `docs/performance/PERFORMANCE_AND_LOAD_TESTING.md`
- `docs/agent-runs/0012-load-client-foundation.md`

Checks:

- Baseline CMake configure/build/ctest set
- `cmake-build-split/battle_load_client.exe --dry-run --clients 2 --duration 1 --command-rate 1 --output reports/load/split-dry-run.md`
- Do not commit generated dry-run report unless it is intentionally part of the slice
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

### 6. `agent/security-secrets-foundation`

Scope:

- Task packets `0011` and `0015`.
- Telegram init data validation.
- Safe logging/redaction and secrets-scan guardrails.

Files:

- `.gitleaks.toml`
- `scripts/agent/validate_agent_harness.py`
- `src/battle_backend/security/README.md`
- `src/battle_backend/security/TelegramAuth.hpp`
- Telegram-auth sections of `src/battle_backend/placeholder.cpp`
- `tests/security/TelegramAuthTests.cpp`
- `docs/agent-runs/0011-telegram-auth-validation.md`
- `docs/agent-runs/0015-secrets-and-safe-logging-guardrails.md`
- `docs/security/SECURITY_REQUIREMENTS.md`
- Telegram-auth/security sections of `docs/telegram/TELEGRAM_SECURITY.md`
- security portions of `CMakeLists.txt`

Checks:

- Baseline CMake configure/build/ctest set
- `python scripts/ci/scan_secrets.py`
- `gitleaks` because `.gitleaks.toml` changes
- `python scripts/agent/validate_agent_harness.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

### 7. `agent/agent-harness-foundation`

Scope:

- Task packets `0013` and `0014`.
- Agent run summaries and harness validation updates not already included in product slices.
- Release integration plan and final report.

Files:

- `docs/agent-runs/0013-ci-cd-guardrails.md`
- `docs/agent-runs/0014-agent-harness-scoped-instructions.md`
- `docs/agent-runs/review-fix-foundation-blockers.md`
- `docs/agent-runs/foundation-through-0015-split-plan.md`
- `docs/agent-runs/foundation-through-0015-release-integration.md`
- any remaining agent-harness-only portions of `scripts/agent/validate_agent_harness.py`

Checks:

- `python scripts/agent/validate_agent_harness.py`
- `python scripts/ci/scan_secrets.py`
- `bash scripts/ci/validate_structure.sh` if available
- `git diff --check`

## Expected Merge Order

1. `agent/core-protocol-foundation`
2. `agent/backend-session-foundation`
3. `agent/transports-foundation`
4. `agent/clients-foundation`
5. `agent/load-observability-foundation`
6. `agent/security-secrets-foundation`
7. `agent/agent-harness-foundation`

No `.github/workflows/**`, `deploy/**`, or `docs/ci/**` changes are present in the safety snapshot, so no separate CI/CD workflow branch is planned.

## Risky Or Mixed Files

- `CMakeLists.txt`: split target/test additions by slice.
- `src/battle_backend/placeholder.cpp`: split session registry implementation from Telegram auth implementation.
- `docs/architecture/PROTOCOL.md`: split shared envelope docs from transport/client notes.
- `docs/architecture/TRANSPORT_ABSTRACTION.md`: split backend session notes from transport notes.
- `docs/telegram/TELEGRAM_SECURITY.md`: split WebSocket transport notes from backend auth validation.
- `scripts/agent/validate_agent_harness.py`: keep only harness/security-validation changes in the security or harness slice, depending on actual diff context.

## Stop Conditions

- Stop before merging any slice whose checks fail.
- Stop if a mixed file cannot be split without breaking a dependency.
- Stop if `master` becomes dirty outside the active slice.
- Stop if any command reveals real secrets, untracked binaries, or generated build output staged for commit.
