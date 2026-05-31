# Run 0070: TCP vertical smoke snapshot budget fix

## Status

State: commit-ready
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0070-tcp-vertical-smoke-idle-timeout-fix.md`
Implementation commit: pending

## Scope

- TCP vertical smoke harness test-only frame/idle/outbound budget configuration and task/run documentation only.
- No source, workflow, deploy, protocol/schema, scenario config, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- The failed 0069 validation indicates a smoke harness budget issue: `tcp_live_tick_smoke.py` passes with a larger frame/idle/outbound budget, while `tcp_vertical_slice_smoke.py` closed a valid creator session before the first full snapshot could be observed.

## Test Impact Matrix

Changed behavior:
- TCP vertical smoke test gives happy-path clients enough test-only frame, idle, and outbound budget for current authoritative snapshots while preserving the short handshake timeout negative case.

Tests added/updated:
- Happy path: `tcp_vertical_slice_smoke.py` now observes snapshots without self-closing valid clients.
- Corner cases: existing malformed JSON, unknown type, oversized frame, and handshake timeout checks remain in the same smoke script.
- Invalid input / hostile input: malformed/unknown/oversized TCP cases remain covered.
- Authority / ownership: no authority behavior changed; test still verifies accepted intention command through the backend.
- Resource bounds / performance: test timeout configuration only; no production runtime limits changed.
- Regression: `tcp_live_tick_smoke.py`, architecture/agent validators, secret scan, and diff check are required.
- Manual UI checks: not applicable.

Not tested and why:
- Full final pre-push suite remains a separate validation task after this blocker is fixed.

## Checks

- `python tests/integration/server/tcp_vertical_slice_smoke.py` - passed after the test-only budget fix.
- `python tests/integration/server/tcp_live_tick_smoke.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warnings only.

## Reviews

- Verification-Agent: approved; required focused checks passed and diff stayed within allowed files.
- Security-Agent: approved; malformed JSON, unknown type, oversized frame, and handshake timeout negative paths remain in the smoke test; no production limits changed.

## Risks

- If snapshots grow beyond the new local smoke budget, the smoke should assert expected snapshot size explicitly rather than silently lowering production limits.
