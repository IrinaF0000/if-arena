# Task 0060: Score, capture, and winner clarity

## Goal

Make live score, scoring events, capture feedback, and final winner text explicit in desktop and mobile clients without changing authoritative game rules.

## Scope

Allowed files:

- `src/battle_qt_client/**`
- `frontend/telegram_mini_app/src/**`
- `tests/frontend/**`
- `docs/agent-runs/0060-score-capture-winner-clarity.md`
- `docs/agent-tasks/0060-score-capture-winner-clarity.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- gameplay rules in `battle_core`
- scenario configs
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- Live score text uses explicit `Blue X - Y Red` wording.
- Score change events identify the scoring team.
- Capture feedback remains visible and score feedback is also rendered.
- Final winner text identifies winner and score.

## Required quality gates

- Gate A. Every implementation task
- Gate D. Qt client
- Gate E. Telegram Mini App
- Gate K. Sequential agent pipeline

## Required checks

- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `node tests/frontend/telegram_arena_canvas_assets.mjs`
- `npm run typecheck`
- `npm run lint`
- `npm run build`
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

- Mobile canvas tests assert score event feedback and winner score text.
- Qt HUD and match-over overlay use explicit score/winner wording.
- No source of gameplay authority moves into clients.

## Rollback note

Revert this task commit to return score/capture/winner text to the previous presentation.
