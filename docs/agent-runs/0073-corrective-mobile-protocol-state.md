# Run 0073: Corrective mobile protocol state

## Status

State: commit-ready
Branch: `codex/corrective-pre-push`
Task packet: `docs/agent-tasks/0073-corrective-mobile-protocol-state.md`
Implementation commit: pending

## Scope

- Telegram Mini App UI state, WebSocket client phase guards, focused frontend/browser tests, and task/run documentation only.
- No C++ gameplay/backend authority, workflow, deploy, protocol/schema, scenario config, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- The reported "invalid protocol" can be caused by the real UI exposing create/join/gameplay actions before authentication/match state and the WebSocket client queueing wrong-phase messages while connecting.
- Local browser testing should continue to use demo auth because Telegram `initData` is absent.

## Test Impact Matrix

Changed behavior:
- Mini App controls are enabled only for the current connection phase.
- WebSocket client rejects wrong-phase create/join/input/rematch locally and queues only auth while connecting.

Tests added/updated:
- Happy path: Playwright e2e verifies two real browser clients connect/authenticate/create/join and receive snapshots.
- Corner cases: WebSocket client test verifies `create_match` is not sent before authentication, including while auth is queued before socket open.
- Invalid input / hostile input: wrong-phase sends stay local and do not produce server protocol errors.
- Authority / ownership: no gameplay state authority moved to the client; the client only gates when intentions may be sent.
- Resource bounds / performance: queue behavior is stricter; non-auth sends are no longer queued before allowed phases.
- Regression: frontend protocol/touch/layout/WebSocket tests, typecheck/lint/build, Playwright e2e, agent validators, secret scan, and diff check passed.
- Manual UI checks: real browser e2e exercises the browser UI; broader manual visual acceptance remains later in the corrective plan.

Not tested and why:
- Real Telegram shell auth was not tested; this task targets local browser/Vite flow and preserves demo auth when Telegram init data is absent.

## Checks

- `node tests/frontend/telegram_websocket_client_behavior.mjs` - passed.
- `node tests/frontend/telegram_main_layout_contract.mjs` - passed.
- `node tests/frontend/telegram_touch_controls_behavior.mjs` - passed.
- `node tests/frontend/telegram_protocol_validation.mjs` - passed.
- `npm.cmd run typecheck` - passed.
- `npm.cmd run lint` - passed.
- `npm.cmd run build` - passed with escalation.
- `cmd /c scripts\run\stop_if_arena.cmd` - passed before and after Playwright e2e.
- `npm.cmd run test:e2e` - passed with escalation.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warnings only.

## Reviews

- Verification-Agent: approved; required checks passed and diff stayed within allowed files.
- Security-Agent: approved; wrong-phase client messages are blocked locally and no auth/protocol trust boundary is weakened.
- Frontend-Agent: approved; controls now reflect connection phase and browser e2e covers real UI connect/create/join/snapshot.

## Risks

- The e2e assertion for post-click disabled create state was removed because authentication can complete before the assertion observes the transient state; stable assertions cover disabled initial state and enabled authenticated state.
