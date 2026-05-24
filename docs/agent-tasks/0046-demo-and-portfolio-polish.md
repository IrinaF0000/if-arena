# Task 0046: Demo assets and portfolio polish

## Goal

Make the repository easy to evaluate as a portfolio project in the first two minutes.

## Scope

Allowed files:

- `README.md`
- `docs/demo/**`
- `docs/agent-runs/0046-demo-and-portfolio-polish.md`
- `docs/agent-tasks/0046-demo-and-portfolio-polish.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- source code
- generated frontend `dist/**`
- release tags
- pushing to GitHub

## Required behavior

- Add portfolio-focused README entry points without renaming the project.
- Add checked-in demo/screenshot assets or a focused demo capture guide.
- Add a concise "What this demonstrates" section covering:
  - C++20 backend;
  - server authority;
  - isolated deterministic core;
  - raw TCP and WebSocket;
  - Qt client;
  - Telegram Mini App client;
  - tests/security/load checks;
  - agent workflow.
- Add an architecture diagram only if it improves reviewer comprehension.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Documentation-only review discipline from `docs/AGENTS.md`

## Required checks

- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes, for documentation accuracy and boundary claims
- Frontend-Agent: no
- Qt-Agent: no
- Security-Agent: no, unless auth, secrets, network input, or deployment instructions change
- Performance-Agent: no, unless load/performance claims change
- Verification-Agent: yes

## Acceptance criteria

- README top section communicates the project value quickly and links to demo material.
- Demo docs include screenshots/demo-loop assets or clear capture instructions.
- No source, CI workflow, deploy, generated dist, tag, or push changes are made.

## Rollback note

Revert this task commit to remove the portfolio/demo polish.
