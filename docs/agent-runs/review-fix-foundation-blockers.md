# Review Fix: Foundation Blockers

Date: 2026-05-20

## Files changed

- `CMakeLists.txt`
- `src/battle_backend/Session.hpp`
- `src/battle_backend/placeholder.cpp`
- `src/battle_transport_ws/WebSocketSession.hpp`
- `src/battle_transport_ws/placeholder.cpp`
- `tests/unit/backend/SessionRegistryTests.cpp`
- `tests/unit/transport/WebSocketSessionTests.cpp`
- `docs/agent-runs/review-fix-foundation-blockers.md`

## Dependency boundary fix

- Added `battle_backend_api` as a narrow interface target exposing backend-facing session abstractions.
- Kept `battle_backend` linked to `battle_core` and `battle_protocol`.
- Changed `battle_transport_tcp` and `battle_transport_ws` to link only `battle_protocol` and `battle_backend_api`, so transport targets no longer pull in `battle_core`.
- `battle_server_app` remains the composition point for backend plus enabled transports.

## CTest registration

CTest now runs 6 tests:

- `battle_core_tests`
- `battle_protocol_tests`
- `battle_backend_tests`
- `battle_transport_tcp_tests`
- `battle_transport_ws_tests`
- `battle_security_tests`

Result: 6/6 passed.

## Related fixes

- `SessionRegistry::createSession()` now returns the created `SessionId`, so callers and tests do not assume allocation order.
- `WebSocketSessionAdapter` no longer stores outbound payload copies in an unbounded vector; it tracks a bounded-size counter for skeleton/test observability.

## Checks run

- `cmake -S . -B cmake-build-review-fix -G Ninja -DBATTLE_BUILD_QT_CLIENT=OFF` - passed.
- `cmake --build cmake-build-review-fix --parallel` - passed.
- `ctest --test-dir cmake-build-review-fix --output-on-failure` - passed, 6 tests run.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed, with CRLF normalization warnings only.

## Remaining blockers

- None known for the reviewed foundation blockers.
