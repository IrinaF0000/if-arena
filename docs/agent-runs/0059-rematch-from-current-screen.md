# Run 0059: Rematch from current screen

## Status

State: commit-ready
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0059-rematch-from-current-screen.md`

## Scope

- Add an authoritative next-match request and desktop/mobile controls.
- Add paired scenario coverage for restarting from the same screen.
- No CI/CD, deploy, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Test Impact Matrix

Changed behavior:
- Client protocol supports `start_next_match` in the in-match phase.
- Backend starts a fresh configured match only after the previous match is finished and requester is a participant.
- Qt and Telegram Mini App expose a next-match control after a finished snapshot.

Tests added/updated:
- Happy path: backend unit and desktop/mobile rematch scenario verify fresh match creation, reset score/objective, and fresh snapshots.
- Corner cases: unfinished match rematch attempts are rejected.
- Invalid input / hostile input: protocol validation rejects authority fields on `start_next_match`.
- Authority / ownership: backend owns rematch creation and scenario state; clients send only finished-match intent.
- Resource bounds / performance: rematch uses existing max-match capacity and bounded outbound queues.
- Regression: pending.
- Manual UI checks: Mini App built output opened in Browser at `http://127.0.0.1:5175/`; the `Next` control rendered disabled before match finish and the page remained usable.

Not tested and why:
- Full manual two-player rematch was not run; automated TCP/WebSocket scenario wrappers cover the same rematch contract end to end.

## Checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `python tests/integration/desktop/rematch_same_screen_desktop.py` - passed.
- `python tests/integration/mobile/rematch_same_screen_mobile.py` - passed.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/mobile/objective_run_full_capture_mobile.py` - passed.
- `python tests/integration/desktop/objective_event_sequence_desktop.py` - passed.
- `python tests/integration/mobile/objective_event_sequence_mobile.py` - passed.
- `node tests/frontend/telegram_websocket_client_behavior.mjs` - passed.
- `node tests/frontend/telegram_protocol_validation.mjs` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python tests/ci/validate_architecture_boundaries_tests.py` - passed.
- `python scripts/ci/validate_no_hardcoded_scenarios.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with approved escalation after sandbox-only Vite/esbuild config path access failed.
- Browser sanity check of built Mini App - passed.
- `git diff --check` - passed with CRLF normalization warnings only.

## Reviews

- Architecture-Agent: approved; rematch remains in protocol/backend/server/client boundaries and does not move transport or UI code into `battle_core`.
- Security-Agent: approved; `start_next_match` is phase-gated, participant-gated, rejects authority fields, and secret scan passed.
- Performance-Agent: approved; finished-match tickers are deactivated when a rematch starts, and capacity/queue bounds remain in force.
- Frontend-Agent: approved; Mini App typecheck/lint/build passed and Browser sanity confirmed disabled pre-finish `Next` control.
- Qt-Agent: approved; Qt client exposes next-match only after finished snapshots and resets command sequence on new `match_joined`.
- Verification-Agent: approved; CTest, frontend checks, desktop/mobile rematch scenarios, existing scenario regressions, and validators passed.

## Risks

- Match records are retained in memory until process restart; existing max-match capacity bounds still apply.
