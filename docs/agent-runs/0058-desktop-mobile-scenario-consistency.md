# Run 0058: Desktop and mobile scenario consistency

## Status

State: commit-ready
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0058-desktop-mobile-scenario-consistency.md`

## Scope

- Scenario metadata propagation, parsing, display, docs, and tests.
- No scenario config, CI/CD, deploy, generated, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Test Impact Matrix

Changed behavior:
- Server-origin `match_joined` and `snapshot` payloads include scenario metadata.
- Desktop and mobile clients parse/show scenario id in service/debug UI.
- Desktop/mobile scenario tests assert metadata consistency and authoritative map/hazard metadata parity.

Tests added/updated:
- Happy path: backend unit test asserts snapshot scenario metadata; desktop/mobile scenario runners assert `match_joined` and snapshot scenario metadata.
- Corner cases: scenario runner compares authoritative map dimensions, obstacle count, hazard ids, and hazard metadata against the referenced game config.
- Invalid input / hostile input: frontend behavior test rejects `match_joined` without scenario metadata and does not enter match state.
- Authority / ownership: metadata is server-origin only; clients display it and still send intentions only.
- Resource bounds / performance: payload expansion is bounded fixed metadata per match/snapshot; existing frame/message limits remain unchanged.
- Regression: CTest, desktop/mobile scenarios, frontend typecheck/lint/build, architecture/secret/agent validators passed.
- Manual UI checks: not run live; Qt and Mini App service UI now include scenario id outside the arena.

Not tested and why:
- Full two-client manual play was not run for this metadata-only phase; automated desktop/mobile TCP/WebSocket scenario checks validate the consistency contract.

## Checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/mobile/objective_run_full_capture_mobile.py` - passed.
- `python tests/integration/desktop/objective_event_sequence_desktop.py` - passed.
- `python tests/integration/mobile/objective_event_sequence_mobile.py` - passed.
- `node tests/frontend/telegram_websocket_client_behavior.mjs` - passed.
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
- `git diff --check` - passed with CRLF normalization warnings only.

## Reviews

- Architecture-Agent: approved; scenario metadata is server-origin DTO content and does not move authority into clients or `battle_core`.
- Security-Agent: approved; no auth/session trust boundary changed, missing metadata is rejected by frontend runtime validation, and secret scan passed.
- Frontend-Agent: approved; strict typecheck/lint/build passed and Mini App shows scenario id in service UI.
- Qt-Agent: approved; Qt parser/network/UI compile under CTest and scenario id is shown in service HUD/identity text.
- Verification-Agent: approved; required checks passed and scenario consistency assertions cover both desktop and mobile transports.

## Risks

- Existing clients built from this tree now require server-origin scenario metadata; this is intentional for the pre-push consistency gate.
