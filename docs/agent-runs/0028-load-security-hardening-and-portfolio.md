# Task 0028: Load, Security Hardening, And Portfolio Docs

State: commit-ready

## Coordinator Preflight

- Branch: `agent/0028-load-security-hardening-and-portfolio`
- Base: clean `master` after task 0027 merge.
- Task packet: `docs/agent-tasks/0028-load-security-hardening-and-portfolio.md`
- Allowed files:
  - `tools/load_client/**`
  - `reports/load/**`
  - `tests/load/**`
  - `tests/security/**`
  - `docs/security/**`
  - `docs/performance/**`
  - `docs/operations/**`
  - `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
  - `README.md`
  - `replays/**`
  - `scripts/** only if non-CI helper script is required`
  - `docs/agent-runs/`
- Forbidden files:
  - `.github/workflows/**`
  - `deploy/**`
  - `src/battle_core/**` except hardening bug fixes
  - `src/battle_qt_client/**` except docs/screenshots
  - `frontend/**` except docs/screenshots
  - `external/**`
- Required reviews: Security-Agent, Performance-Agent, Verification-Agent.

## Implementation Assumptions

- No backend or transport source changes are required for this final hardening packet.
- Load testing should be reproducible and honest about local-only limits.
- Security tests should exercise the built server through raw TCP without committing secrets or generated artifacts.

## Implementation Note

Changed files:

- `tools/load_client/LoadClientMain.cpp`
- `tools/load_client/README.md`
- `tests/load/load_client_dry_run.py`
- `tests/load/local_tcp_load_scenarios.py`
- `tests/security/tcp_protocol_negative.py`
- `reports/load/0028-local-tcp-smoke.md`
- `reports/load/README.md`
- `docs/performance/PERFORMANCE_AND_LOAD_TESTING.md`
- `docs/security/THREAT_MODEL.md`
- `docs/security/SECURITY_REQUIREMENTS.md`
- `docs/operations/OBSERVABILITY.md`
- `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`
- `README.md`
- `docs/agent-runs/0028-load-security-hardening-and-portfolio.md`

Summary:

- Expanded `battle_load_client --dry-run` to validate named scenarios: `connect_only`, `create_and_join`, `gameplay`, `slow_readers`, `command_spam`, and `invalid_payloads`.
- Added bounded seed/client/duration/rate validation and richer markdown report fields.
- Added `tests/load/load_client_dry_run.py` for dry-run scenario and bounds coverage.
- Added `tests/load/local_tcp_load_scenarios.py` for a small live raw TCP smoke covering create/join/gameplay, duplicate command rejection, malformed JSON, unknown type, and report generation.
- Added `tests/security/tcp_protocol_negative.py` for command-before-auth and forbidden authority field rejection through the server.
- Added reproducible local report `reports/load/0028-local-tcp-smoke.md`.
- Updated security, performance, operations, plan, and README docs with quickstarts, limitations, remaining risks, and portfolio summary.

Known risks:

- The live load report is intentionally tiny and local. It validates scenario wiring and negative behavior, not production capacity.
- Large slow-reader soak, mixed TCP/WebSocket load, public deployment, TLS/WSS operations, snapshot coalescing, and Telegram replay protection remain future work.

Ready for verification: yes.

## Test Impact Matrix

Changed behavior:

- Load client dry-run now validates all hardening scenario names and writes richer bounded reports.
- Local load/security tests exercise raw TCP server behavior with normal, spam/duplicate, malformed, invalid-order, and authority-violation clients.
- Docs now describe implemented quickstarts, reproducible reports, and honest limitations.

Tests added/updated:

- Happy path: `tests/load/local_tcp_load_scenarios.py` authenticates two TCP clients, creates/joins a match, receives snapshots, sends gameplay commands, and records latency.
- Corner cases: `tests/load/load_client_dry_run.py` checks all scenario names, seed output, and report fields.
- Invalid input / hostile input: `tests/load/local_tcp_load_scenarios.py` sends malformed JSON and unknown message type; `tests/security/tcp_protocol_negative.py` sends create-before-auth.
- Authority / ownership: `tests/security/tcp_protocol_negative.py` sends forbidden `playerId` and `hp` fields in an input command and expects server rejection.
- Resource bounds / performance: dry-run test checks oversized client count rejection; load report records command/rejection/protocol-error counts and latency percentiles.
- Regression: full CTest suite remains green.
- Manual UI checks: none changed.

Not tested and why:

- Large slow-reader soak is documented but not run locally to keep the final packet reproducible and quick.
- Mixed TCP/WebSocket load is documented as future work.

## Verification

Commands run:

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - pass.
- `cmake --build build --parallel` - pass.
- `ctest --test-dir build --output-on-failure` - pass, 7/7 tests.
- `python tests/load/load_client_dry_run.py` - pass.
- `python tests/load/local_tcp_load_scenarios.py` - pass; wrote `reports/load/0028-local-tcp-smoke.md`.
- `python tests/security/tcp_protocol_negative.py` - pass.
- `git diff --check` - pass.
- `python scripts/ci/scan_secrets.py` - pass.
- `python scripts/agent/validate_agent_harness.py` - pass.
- `python scripts/agent/validate_playable_task_sequence.py` - pass.
- `./scripts/ci/validate_structure.sh` through Git Bash with bundled Python shim - pass.

## Reviews

Security-Agent:

Decision: approve

Findings:

- Negative TCP security tests cover malformed JSON, unknown message type, invalid message order, and forbidden authority fields.
- Docs explicitly call out demo auth, Telegram replay protection gap, raw TCP exposure risk, and no-secrets requirements.

Required re-checks:

- Keep `python scripts/ci/scan_secrets.py` in final verification.

Performance-Agent:

Decision: approve-with-minors

Findings:

- Load report is honest and bounded; it avoids production throughput claims.
- Slow-reader and mixed-transport large soaks are documented as future work rather than overstated.

Required re-checks:

- Future release hardening should add larger slow-reader and mixed TCP/WebSocket live scenarios after async/scaling work.

Verification-Agent:

Decision: approve

Findings:

- Scope stays within task-allowed files.
- Test Impact Matrix is present.
- Required checks pass.

Required re-checks:

- None.

## Commit

Pending scoped commit.
