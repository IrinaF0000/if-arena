# Task 0070: TCP vertical smoke snapshot budget fix

## Goal

Fix the local TCP vertical-slice smoke harness so the happy two-client flow has enough test-only frame, idle, and outbound budget to observe authoritative snapshots before negative protocol cases run.

## Scope

Allowed files:

- `tests/integration/server/tcp_vertical_slice_smoke.py`
- `docs/agent-runs/0070-tcp-vertical-smoke-idle-timeout-fix.md`
- `docs/agent-tasks/0070-tcp-vertical-smoke-idle-timeout-fix.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `deploy/**`
- `docs/ci/**`
- source code
- frontend source code
- protocol/schema changes
- scenario config changes
- generated build outputs
- release tags
- pushing to GitHub
- merging into `master` or `main`

## Required behavior

- TCP vertical smoke happy path no longer fails because the test's own frame, idle, or outbound limits close a valid snapshot stream.
- Negative malformed, unknown type, oversized frame, and handshake timeout checks remain covered.
- No production defaults, protocol behavior, backend behavior, or gameplay behavior change.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Network/protocol test quality rules

## Required checks

- `python tests/integration/server/tcp_vertical_slice_smoke.py`
- `python tests/integration/server/tcp_live_tick_smoke.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `python scripts/ci/scan_secrets.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: no
- Security-Agent: yes
- Performance-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Both TCP smoke scripts pass against the local `build/battle_server_app.exe`.
- Diff is limited to the test harness and task/run documentation.
- Required checks pass or any skipped check is explicitly justified.

## Rollback note

Revert this task commit to restore the previous TCP vertical smoke timeout configuration.
