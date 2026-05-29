# Run 0057: Pre-push baseline inspection

## Status

State: commit-ready
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0057-pre-push-baseline-inspection.md`

## Scope

- Baseline inspection for `if_arena_next_pre_push_agent_plan.md`.
- No source, scenario config, frontend source, CI/CD, deploy, generated, push, tag, or merge changes.

## Baseline State

- Current branch: `agent/0047-playable-stabilization`.
- Current HEAD: `31a3f4c`.
- Uncommitted files before this task: untracked `if_arena_next_pre_push_agent_plan.md` and `post_0046_playable_stabilization_agent_plan.md`.
- Generated files present: ignored build directories, frontend `node_modules/` and `dist/`, Python `__pycache__/`, and ignored local `config/examples/server.ws.local.json`.

## Test Impact Matrix

Changed behavior:
- None. Baseline documentation only.

Tests added/updated:
- Happy path: not applicable; no runtime behavior changed.
- Corner cases: not applicable; no runtime behavior changed.
- Invalid input / hostile input: not applicable; no input handling changed.
- Authority / ownership: baseline validators will be run.
- Resource bounds / performance: no behavior changed; baseline build/test checks will be run.
- Regression: baseline checks will be recorded.
- Manual UI checks: visual baseline evidence will be captured or referenced where practical.

Not tested and why:
- Live desktop GUI play was not performed in Phase 0; baseline uses current automated Qt checks plus prior visual-readability context.

## Checks

- `git status --short --ignored` - recorded.
- `git log --oneline -10` - recorded; HEAD was `31a3f4c test: record final validation gate`.
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug` - passed.
- `cmake --build build --parallel` - passed, no work to do.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `bash scripts/ci/validate_structure.sh` - passed with Git Unix tools first in `PATH` and `python3` shimmed to local Python.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python tests/ci/validate_architecture_boundaries_tests.py` - passed.
- `python scripts/ci/validate_no_hardcoded_scenarios.py` - passed.
- `python scripts/ci/validate_gameplay_scenario_pairs.py` - passed.
- `npm.cmd install` - initial sandbox run timed out; approved escalated rerun passed and reported 2 moderate audit vulnerabilities.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - initial sandbox run failed on Vite/esbuild config path access; approved escalated rerun passed.

## Visual Baseline

- Telegram Mini App baseline captured in the in-app Browser from local Vite on `http://127.0.0.1:5175/`.
- Observed first screen: arena is visible, but service controls/status remain around the game area, the base zones are visually large, and the screen still reads partly like a debug/control prototype. This supports the next plan's desktop/mobile UI cleanup and side-panel goals.
- Desktop GUI screenshot was not captured in this task; desktop baseline is represented by the current Qt build/tests and the existing post-0054 visual-readability notes.

## Reviews

- Verification-Agent: approved; baseline checks passed after documented environment reruns, no source files changed, and generated artifacts remain unstaged/ignored.

## Risks

- `npm install` still reports 2 moderate audit vulnerabilities; this remains a dependency/security follow-up rather than an automatic forced upgrade.
