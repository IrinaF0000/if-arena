# Task 0075: Corrective SVG asset verification

## Goal

Make mandatory desktop and Mini App SVG asset loading explicit and testable so missing or invalid visual assets fail automated checks.

## Scope

Allowed files:

- `CMakeLists.txt`
- `src/battle_qt_client/tests/**`
- `frontend/telegram_mini_app/tests/e2e/**`
- `docs/agent-runs/0075-corrective-svg-asset-verification.md`
- `docs/agent-tasks/0075-corrective-svg-asset-verification.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay rules
- protocol/schema changes
- scenario config changes
- generated build outputs
- generated dependency directories
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Qt test fails if any mandatory desktop SVG resource is missing or invalid.
- Playwright e2e fails if any mandatory Mini App SVG URL is not served with HTTP 200 and `image/svg+xml`.
- Existing browser gameplay e2e remains stable when snapshots arrive before transient labels are visible.
- No gameplay authority or protocol behavior changes.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- TypeScript/frontend quality rules
- Qt review for resource test coverage

## Required checks

- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH=<local Qt path>`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure -R battle_qt_client`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm.cmd run typecheck`
- `npm.cmd run lint`
- `npm.cmd run build`
- `npm.cmd run test:e2e`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: no
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: yes
- Qt-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Desktop mandatory SVGs are verified through Qt resources and `QSvgRenderer::isValid()`.
- Mini App mandatory SVGs are verified through real browser HTTP fetches.
- Required checks pass or any skipped check is explicitly justified.

## Rollback note

Revert this task commit to remove the explicit SVG asset verification tests.
