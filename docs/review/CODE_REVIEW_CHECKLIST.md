# Code Review Checklist

## Architecture

- [ ] Does the change respect dependency direction?
- [ ] Is `battle_core` free from networking/UI/deployment concerns?
- [ ] Is protocol transport-independent?
- [ ] Are transports only adapters?
- [ ] Does backend remain authoritative?
- [ ] Are public APIs small and cohesive?
- [ ] Is ownership/lifetime clear?

## C++ quality

- [ ] C++20 style is used appropriately.
- [ ] RAII is used.
- [ ] No C-style casts.
- [ ] No raw owning pointers.
- [ ] No hidden mutable globals.
- [ ] No unnecessary shared mutable state.
- [ ] Error handling is explicit.
- [ ] Hot paths avoid unnecessary allocations where reasonable.
- [ ] Threading assumptions are documented.

## Security

- [ ] All network input is validated.
- [ ] Size limits exist and are enforced.
- [ ] Rate limits exist where needed.
- [ ] Slow clients cannot grow memory unbounded.
- [ ] Logs do not leak secrets or raw auth data.
- [ ] Telegram raw `initData` is validated on backend.
- [ ] `initDataUnsafe` is not trusted.
- [ ] Client cannot claim authoritative game state.
- [ ] Negative tests cover hostile cases.

## TCP

- [ ] Partial reads handled.
- [ ] Combined frames handled.
- [ ] Oversized frames rejected early.
- [ ] Disconnect paths are safe.
- [ ] Write queue is bounded.
- [ ] Ping/idle timeout considered.

## WebSocket

- [ ] Message size limit enforced.
- [ ] WSS assumed for public deployment.
- [ ] Malformed messages handled.
- [ ] Slow browser clients handled.
- [ ] Auth flow is safe.

## Gameplay fairness

- [ ] MVP map remains 21x13 unless changed by ADR.
- [ ] Map uses 180-degree rotational symmetry.
- [ ] Player stats and start conditions remain equal.
- [ ] Carrier slowdown, drop-on-hit, and pickup lock are server-authoritative.
- [ ] Objective pickup/drop/capture cannot be claimed by the client.
- [ ] Player-oriented view is client-side only and does not affect server authority.
- [ ] Replay/debug canonical orientation remains possible.

## Qt

- [ ] UI thread is not blocked.
- [ ] Network state separated from widgets.
- [ ] Signals/slots are clear.
- [ ] Errors are user-visible.
- [ ] UI readability rules followed.

## TypeScript

- [ ] Strict TypeScript.
- [ ] No unchecked `any`.
- [ ] Inbound messages treated as unknown.
- [ ] No secrets in frontend.
- [ ] No unsafe DOM insertion.
- [ ] WebSocket lifecycle is explicit.
- [ ] Touch and desktop input considered.

## Performance

- [ ] Resource growth is bounded.
- [ ] Metrics updated.
- [ ] Load impact considered.
- [ ] No O(N^2) broadcast path without justification.
- [ ] Tick loop cost considered.


## CI/CD and workflow preservation

- [ ] Workflow files were not changed unless the task explicitly allowed it.
- [ ] PR CI remains validation-only and does not deploy.
- [ ] PR CI does not use production secrets.
- [ ] Main CI remains separate from PR CI.
- [ ] Workflow permissions are least-privilege.
- [ ] `pull_request_target` was not introduced without explicit security review.
- [ ] Required check names are documented if changed.
- [ ] `docs/ci/CI_CD_GUARDRAILS.md` is updated if CI/CD behavior changed.
- [ ] Rollback note exists for workflow changes.

## Tests

- [ ] Unit tests added where appropriate.
- [ ] Integration tests added for networking/backend flows.
- [ ] Negative tests added for security-sensitive paths.
- [ ] Load or stress test updated when scalability behavior changes.
- [ ] Manual checklist updated for UI behavior.

## Docs

- [ ] Relevant docs updated.
- [ ] ADR added for significant decisions.
- [ ] Protocol docs updated for message changes.
- [ ] Security docs updated for trust-boundary changes.
- [ ] Implementation plan/backlog updated if scope changes.


## Secrets and logging

- No real `.env` values, Telegram bot tokens, session signing keys, TLS keys, cloud credentials, or personal access tokens are committed.
- Auth payloads, Telegram `initData`, session tokens, and raw frames are not logged.
- Secret scanning is kept in PR and main CI.
- Any auth/logging/deploy change has Security Review Agent approval.
