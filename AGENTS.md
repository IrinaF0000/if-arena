# Agent Entry Point

This repository is designed for scoped coding-agent work. Keep this file short. Use it as a routing layer, not as a full manual.

## First steps for every agent

1. Read `PROJECT_MAP.md` and the task packet assigned to you.
2. Read the nearest `AGENTS.md` in the directory you will change.
3. Read only the focused rule files relevant to the task:
   - `docs/agent-rules/security/NETWORK_INPUT.md`
   - `docs/agent-rules/security/TELEGRAM_AUTH.md`
   - `docs/agent-rules/quality/CPP.md`
   - `docs/agent-rules/quality/QT.md`
   - `docs/agent-rules/quality/TYPESCRIPT.md`
   - `docs/agent-rules/process/TOKEN_ECONOMY.md`
   - `docs/agent-rules/process/ACCESS_BOUNDARIES.md`
4. Use repo-local skills from `.agents/skills/` only when they match your task.
5. Work only in files allowed by the task packet.

## Non-negotiable rules

- Do not change `.github/workflows/**`, `scripts/ci/**`, `deploy/**`, or `docs/ci/**` unless the task packet explicitly allows CI/CD work.
- Do not put TCP, WebSocket, Qt, Telegram, or deployment code into `battle_core`.
- Do not let clients own authoritative game state. Clients send intentions only.
- Treat all network, Telegram, config, and replay input as untrusted.
- Do not commit real secrets. Use `.env.example` placeholders only and run `python3 scripts/ci/scan_secrets.py` before PRs.
- Add negative tests for networking, parsing, auth, rate limits, and resource bounds.
- Keep diffs small and reviewable.

## Build and check basics

```bash
cmake -S . -B build -DBATTLE_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
bash scripts/ci/validate_structure.sh
python3 scripts/agent/validate_agent_harness.py
```

For frontend changes:

```bash
cd frontend/telegram_mini_app
npm install
npm run typecheck
npm run lint
npm run build
```

## Reporting

At the end of a task, write a short progress note in `docs/agent-runs/` or update the task packet status. Do not paste full files. List changed files, checks run, risks, and follow-up tasks.
