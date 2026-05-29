# Run 0063: Object taxonomy and danger semantics

## Status

State: committed
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0063-object-taxonomy-and-danger-semantics.md`
Implementation commit: `5e5c7f6`

## Scope

- Snapshot metadata, client validation/rendering, focused protocol/docs/tests.
- No gameplay rules, scenario redesign, CI/CD, deploy, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Assumptions

- Phase 6 is presentation/protocol metadata work; authoritative collision, hazard effects, cooldowns, capture, and scoring remain unchanged.
- The current snapshot-only metadata model is preserved until a later static arena metadata channel is justified.

## Test Impact Matrix

Changed behavior:
- Backend snapshots now expose obstacle semantic metadata and hazard visual/drop/range/blocking/ownership metadata.
- Telegram Mini App and Qt client parsers require the new semantic fields before rendering snapshots.
- Telegram Mini App and Qt arena renderers draw clearer blockers plus compact hazard damage/drop markers from metadata.
- Gameplay scenario navigation avoids configured drop-hazard influence while carrying the objective.
- Protocol/scenario docs define the current object taxonomy.

Tests added/updated:
- Happy path: backend snapshot serialization assertions, frontend protocol/canvas fixtures, desktop/mobile gameplay scenarios.
- Corner cases: scenario runner uses configured hazard radius/range when navigating with a carried objective.
- Invalid input / hostile input: Telegram protocol validation rejects inconsistent hazard `causesDrop` metadata and malformed obstacle metadata.
- Authority / ownership: clients only validate/render server-origin metadata; no client-owned HP, score, objective, movement, cooldown, or damage rules were added.
- Resource bounds / performance: backend slow-session test byte ceiling was adjusted for larger snapshots while still asserting bounded queue closure; rendering remains bounded by authoritative obstacle/hazard counts.
- Regression: CTest, paired gameplay scenarios, protocol validation, frontend lint/typecheck/build, scenario validators, architecture validators, secret scan, and diff check passed.
- Manual UI checks: Browser smoke loaded the built Mini App at `http://127.0.0.1:4175/`, confirmed title `IF Arena`, canvas `960x540`, expected controls, and no forbidden/error page.

Not tested and why:
- Live two-human desktop/browser play was not performed; automated TCP/WebSocket scenario tests and Browser static smoke cover this metadata/rendering change.

## Checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH='C:\Qt\6.11.1\mingw_64'` - passed.
- `cmake --build build --parallel` - passed, including `battle_qt_client.exe`.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `node tests/frontend/telegram_protocol_validation.mjs` - passed.
- `node tests/frontend/telegram_websocket_client_behavior.mjs` - passed.
- `node tests/frontend/telegram_arena_canvas_assets.mjs` - passed.
- `npm.cmd install --no-audit --no-fund` - passed with escalation after sandbox timeouts.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with escalation after Vite/esbuild sandbox access denial.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/mobile/objective_run_full_capture_mobile.py` - passed.
- `python tests/integration/desktop/objective_event_sequence_desktop.py` - passed.
- `python tests/integration/mobile/objective_event_sequence_mobile.py` - passed.
- `python tests/integration/desktop/rematch_same_screen_desktop.py` - passed.
- `python tests/integration/mobile/rematch_same_screen_mobile.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python tests/ci/validate_architecture_boundaries_tests.py` - passed.
- `python scripts/ci/validate_no_hardcoded_scenarios.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `python scripts/ci/validate_scenario_map_fairness.py` - passed.
- `python tests/ci/validate_scenario_map_fairness_tests.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `bash scripts/ci/validate_structure.sh` via Git Bash with `python3` shim - passed.
- `git diff --check` - passed with line-ending normalization warnings only.
- Browser smoke of built Mini App - passed; favicon/static 404 was non-blocking and app canvas/control surface loaded.

## Reviews

- Verification-Agent: approved; required checks passed after rerunning gameplay scenarios without shared-port contention.
- Architecture-Agent: approved; metadata stays in backend snapshots, `battle_core` remains isolated, and no client becomes authoritative.
- Security-Agent: approved; inbound Mini App and Qt snapshot parsers reject missing/malformed semantic metadata before rendering.
- Performance-Agent: approved; snapshot size grows by bounded fields per obstacle/hazard and renderer work remains O(authoritative objects).
- Frontend-Agent: approved; strict typecheck/lint/build passed and Browser smoke confirmed the built app loads.
- Qt-Agent: approved; Qt parser/rendering changes compile in `battle_qt_client`, use validated snapshots, and keep rendering separate from network state.

## Risks

- Object visuals are clearer but still code-drawn; Phase 7 should replace bots/hazards/blockers with shared SVG assets.
- Browser smoke did not connect to a live server; live scenario behavior is covered by desktop/mobile TCP/WebSocket scenario runners.
