# Run 0077: Corrective arena overlay cleanup

## Status

State: commit-ready
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0077-corrective-arena-overlay-cleanup.md`
Implementation commit: pending

## Scope

- Desktop Qt side-panel hazard text, Mini App drawer hazard text, canvas/arena overlay removal, focused tests, task/run documentation.
- No workflow, deploy, gameplay authority, protocol/schema, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- Match-over overlay and short event feedback labels are allowed gameplay feedback.
- Long service/status and hazard legend text should live outside the arena.

## Test Impact Matrix

Changed behavior:
- Desktop Qt no longer draws the hazard legend panel over `ArenaView`.
- Desktop side panel now shows hazard summary text beside connection/status/log information.
- Mini App canvas no longer draws active-match HUD/status or hazard legend text over the arena.
- Mini App uses an `Arena info` drawer outside the canvas for hazard summary text.

Tests added/updated:
- Happy path: Qt `ArenaView` test verifies hazard summary text is produced for the side panel; frontend layout contract verifies the out-of-canvas drawer.
- Corner cases: Qt test collapses duplicate visual hazard entries; canvas test asserts old score/status/legend text is absent from active arena rendering.
- Invalid input / hostile input: not applicable; no parser/auth/network behavior changed.
- Authority / ownership: no gameplay authority changed; clients still render server snapshot metadata only.
- Resource bounds / performance: hazard summary remains bounded by unique hazard visual ids; canvas work is reduced by removing two text panels.
- Regression: Qt client tests, frontend canvas/layout tests, typecheck/lint/build, agent validators, secret scan, and diff check passed.
- Manual UI checks: final screenshot/manual acceptance remains in the next corrective validation phase.

Not tested and why:
- Full visual screenshot acceptance is deferred to Phase 7/8, where both desktop and mobile real UI flows are checked from a clean process state.

## Checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=C:\Qt\6.11.1\mingw_64` - passed with optional Vulkan header warning only.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure -R battle_qt_client` - passed, 4/4 tests.
- `node tests/frontend/telegram_arena_canvas_assets.mjs` - passed.
- `node tests/frontend/telegram_main_layout_contract.mjs` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with escalation after sandboxed Vite/esbuild config access failed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warnings only.

## Reviews

- Verification-Agent: approved; required checks passed and diff stayed inside task scope.
- Frontend-Agent: approved; long canvas HUD/legend text moved to DOM drawer while topbar keeps score/status.
- Qt-Agent: approved; hazard legend text is side-panel data and `ArenaView` no longer paints it over the board.

## Risks

- Real screenshot evidence is still required by later corrective acceptance gates.
