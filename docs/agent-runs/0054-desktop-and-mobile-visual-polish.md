# Run 0054: Desktop and mobile visual polish

## Status

State: committed
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0054-desktop-and-mobile-visual-polish.md`

## Scope

- Presentation-only visual polish for Telegram Mini App and Qt arena rendering.
- No gameplay rule, scenario, protocol, workflow, deploy, tag, push, or merge changes.

## Progress

- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: completed.

## Test Impact Matrix

Changed behavior:
- Telegram Mini App and Qt arena rendering now use a warmer tactical palette with clearer board borders, bases, obstacles, objective, and player accents.
- Player visual tokens are larger in both clients without changing authoritative collision or gameplay state.
- Visual readability docs and Qt manual checklist now reflect the post-0054 presentation target.

Tests added/updated:
- Happy path: frontend typecheck/lint/build, Qt MinGW build/CTest, Browser visual smoke screenshot, paired desktop/mobile gameplay scenarios.
- Corner cases: existing Qt transform tests and paired scenarios preserve player-oriented view behavior.
- Invalid input / hostile input: None expected; no parser/security boundary changed.
- Authority / ownership: visual-only changes consume snapshots/events and do not mutate server-owned position, HP, objective, score, cooldown, or match state.
- Resource bounds / performance: rendering still draws a bounded number of players, hazards, obstacles, HUD panels, and labels per frame.
- Regression: CTest, gameplay scenarios, frontend checks, Qt checks, architecture/secret/agent validators passed.
- Manual UI checks: Browser visual smoke opened the Telegram Mini App on local Vite and captured a full-page screenshot; live two-client manual play was not performed.

Not tested and why:
- Full interactive two-client Qt/manual browser play was not run; automated protocol scenarios and Qt/frontend builds cover the changed code paths.

## Checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/mobile/objective_run_full_capture_mobile.py` - passed.
- `python tests/integration/desktop/objective_event_sequence_desktop.py` - passed.
- `python tests/integration/mobile/objective_event_sequence_mobile.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with approved escalation after sandbox-only esbuild path access failed.
- Qt MinGW configure with `BATTLE_BUILD_QT_CLIENT=ON` - passed.
- Qt MinGW build - passed.
- `ctest --test-dir build-qt-mingw --output-on-failure` - passed, 8/8 tests.
- Browser visual smoke: opened `http://127.0.0.1:5174`, captured `0054-mini-app-visual-smoke.png`, then stopped the Vite dev server.
- `git diff --check` - passed with CRLF normalization warnings only.

## Reviews

- Architecture-Agent: approved; client rendering remains presentation-only and no boundaries changed.
- Frontend-Agent: approved; strict typecheck/lint/build passed, Browser visual smoke completed, and text fits current controls.
- Qt-Agent: approved; Qt MinGW build and CTest passed after renderer color/size changes.
- Verification-Agent: approved; required checks passed.

## Risks

- No merge to `master` or `main`, push, or tag was performed.
