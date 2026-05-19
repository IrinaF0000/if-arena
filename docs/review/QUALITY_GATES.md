# Quality Gates

## Gate A. Every implementation task

Required:

- Builds or explains why build is not applicable.
- Relevant tests added/updated or explicitly justified.
- No unrelated changes.
- No C-style casts in new C++ code.
- No raw owning pointers in new code.
- No hidden global mutable state.
- Documentation updated when behavior, API, protocol, architecture, or workflow changes.
- `git diff --check` passes.
- Agent progress updated.
- Token budget rules followed.

## Gate B. Networking and protocol

Required for TCP, WebSocket, protocol, parser, serializer, session, and transport changes:

- Protocol docs updated.
- Message/frame size limits enforced.
- Partial TCP frames handled.
- Combined TCP frames handled.
- Oversized input rejected before unbounded allocation.
- Unknown/malformed messages handled safely.
- Parser errors do not crash server.
- Negative tests added.
- Security-Agent review required.

## Gate C. Backend/gameplay authority

Required for backend, match, command, tick, gameplay, hazards, objective changes:

- Server remains authoritative.
- Client cannot set position, HP, cooldown, damage, score, team, or result.
- Ownership checks exist.
- Command validation tests added.
- Match state mutation confined to owning worker or documented safe path.
- Replay/determinism impact assessed.
- Architecture-Agent review required if public API changes.

Additional required checks for gameplay/fairness changes:

- 21x13 MVP map assumptions remain documented if changed.
- 180-degree rotational symmetry is preserved or the change has a gameplay ADR.
- Equal player starting stats are preserved unless explicitly changed.
- Carrier slowdown, drop-on-hit, and pickup-lock behavior are tested if touched.
- Player-oriented view changes include coordinate transform tests.
- Replay/debug canonical orientation remains available.

## Gate D. Qt client

Required for Qt changes:

- No blocking network calls on UI thread.
- Network code separated from widgets.
- UI does not own authoritative rules.
- Connection errors shown gracefully.
- Input mapping documented.
- Manual UI checklist updated.
- Qt-Agent or Verification-Agent review required.

## Gate E. Telegram Mini App

Required for frontend changes:

- TypeScript strict mode passes.
- ESLint passes or deviations documented.
- No secrets in frontend.
- No trust in `initDataUnsafe`.
- WebSocket inbound messages validated.
- UI handles disconnect/reconnect states.
- Mobile/touch and desktop controls considered.
- Frontend-Agent review required.
- Security-Agent required if auth/network code changes.

## Gate F. Security-sensitive changes

Required for auth, Telegram init data, session identity, logging, config, deployment:

- Threat model updated if assumptions changed.
- Security requirements updated if needed.
- Secrets are not committed.
- Sensitive logs redacted.
- Fail-closed behavior tested.
- Negative tests include hostile inputs.
- Security-Agent review required.

## Gate G. Performance/load changes

Required for queues, workers, broadcasts, metrics, load client, tick loop:

- Every growing resource has a bound.
- No thread-per-client design.
- Slow clients handled.
- Metrics added or updated.
- Load scenario updated if relevant.
- Performance-Agent review required.

## Gate H. Documentation-only changes

Required:

- No unverified technical claims.
- Links and paths are correct.
- Docs do not contradict project map.
- Agent instructions remain actionable and scoped.


## Gate I. CI/CD and workflow changes

Required for changes touching `.github/workflows/**`, `.github/actions/**`, `.github/CODEOWNERS`, `scripts/ci/**`, `deploy/**`, or `docs/ci/**`:

- Dedicated CI/CD task packet exists.
- `docs/ci/CI_CD_GUARDRAILS.md` was read and remains consistent.
- PR CI remains validation-only and does not deploy.
- PR CI does not use production secrets.
- Main CI remains separate from PR CI.
- Workflow `permissions` are least-privilege.
- No `pull_request_target` is introduced without explicit Security-Agent approval.
- Required checks are documented if names change.
- Rollback note is documented.
- Coordinator review required.
- Security-Agent review required.
- Verification-Agent review required.


## Gate J: Secrets, Auth, and Logging Changes

Required for any task that touches auth, Telegram validation, environment loading, logging, deployment secrets, workflow secrets, or redaction helpers.

- `python3 scripts/ci/scan_secrets.py` passes.
- No real tokens, keys, certificates, auth headers, or signed payloads are committed.
- `.env.example` contains placeholders only.
- Frontend environment files do not include backend secrets or Telegram bot tokens.
- Logs use structured categories and redacted values.
- Security Review Agent approval is required before merge.
