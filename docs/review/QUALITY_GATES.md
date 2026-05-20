# Quality Gates

Canonical completion gates. Task packets list which gates apply; do not copy gate text into task packets.

## Gate A. Every implementation task

- Build passes or non-applicability is documented.
- `docs/agent-rules/quality/TESTING.md` is followed.
- Test Impact Matrix is present in the run note.
- Relevant tests/manual checks are added or a specific exception is approved.
- No unrelated changes, hidden global mutable state, raw owning pointers, C-style casts in new C++ code, real secrets, generated dependency directories, or build outputs.
- Docs are updated only when behavior, API, protocol, architecture, workflow, or run instructions change.
- `git diff --check` and required agent/secret checks pass.

## Gate B. Networking and protocol

Required for TCP, WebSocket, protocol, parser, serializer, session, and transport changes:

- Protocol docs updated if externally visible behavior changes.
- Message/frame limits are enforced before unbounded allocation.
- Partial frames, combined frames, malformed messages, unknown types, and parser errors are handled safely.
- Negative tests exist for invalid and hostile input.
- Security-Agent review required.

## Gate C. Backend/gameplay authority

Required for backend, match, command, tick, gameplay, hazards, and objective changes:

- Server remains authoritative for position, HP, cooldowns, damage, score, team, objective state, and match result.
- Ownership and command-validation tests exist.
- Match state mutation stays in the owning worker or a documented safe path.
- Determinism/replay impact is assessed.
- Gameplay fairness is preserved: 21x13 MVP assumptions, 180-degree symmetry, equal starting stats, canonical replay/debug view.
- Carrier slowdown, drop-on-hit, pickup lock, capture, and player-oriented transforms are tested when touched.
- Architecture-Agent review required for public API or boundary changes.

## Gate D. Qt client

- UI thread is not blocked by networking.
- Network code is separate from widgets.
- UI sends intentions only and does not own authoritative rules.
- Errors and disconnects are visible and non-crashing.
- Input mapping and manual checklist are updated.
- Qt-Agent or Verification-Agent review required.

## Gate E. Telegram Mini App

- TypeScript strict mode, lint, and build pass.
- No frontend secrets and no authority based on `initDataUnsafe`.
- WebSocket opens before auth is sent.
- Inbound messages are runtime-validated.
- Disconnect/reconnect and mobile/touch states are handled.
- Frontend-Agent review required; Security-Agent review required for auth/network changes.

## Gate F. Security-sensitive changes

Required for auth, Telegram init data, session identity, logging, config, deployment, and hostile input paths:

- Threat model/security requirements updated if assumptions changed.
- Fail-closed behavior and hostile-input tests exist.
- Sensitive logs are redacted.
- `python3 scripts/ci/scan_secrets.py` passes.
- Security-Agent review required.

## Gate G. Performance/load changes

Required for queues, workers, broadcasts, metrics, load client, tick loop, and resource limits:

- Every growing resource has an explicit bound.
- No thread-per-client or blocking hot path is introduced.
- Slow clients and spam clients are handled.
- Metrics and load scenarios are updated when relevant.
- Performance-Agent review required.

## Gate H. Documentation-only changes

- Claims are accurate and match the code.
- Links and paths are valid.
- Docs do not duplicate canonical rule text.
- Agent instructions remain short, scoped, and executable.

## Gate I. CI/CD and workflow changes

Required for `.github/**`, `scripts/ci/**`, `deploy/**`, or `docs/ci/**` changes:

- Dedicated CI/CD task packet exists.
- PR CI remains validation-only and does not use production secrets.
- Main CI remains separate from PR CI.
- Workflow permissions are least-privilege.
- No `pull_request_target` without explicit Security-Agent approval.
- Required checks and rollback are documented.
- Coordinator, Security-Agent, and Verification-Agent reviews required.

## Gate J. Secrets, auth, and logging

Required for auth, Telegram validation, environment loading, logging, deployment secrets, workflow secrets, and redaction helpers:

- No real tokens, keys, certificates, auth headers, or signed payloads are committed.
- `.env.example` contains placeholders only.
- Frontend env files never contain backend secrets or Telegram bot tokens.
- Logs are structured and redacted.
- Security-Agent approval required before merge.

## Gate K. Sequential agent pipeline

Required for every playable-game task and non-trivial role-based implementation task:

- Task packet and run note exist.
- State moved through `ready -> implementing -> verifying -> reviewing -> fixing -> re-verifying -> commit-ready -> committed`, skipping only states that truly do not apply.
- Coordinator preflight happened before implementation.
- Implementation-Agent did not commit.
- Verification-Agent recorded exact commands, results, and Test Impact Matrix status.
- Required Review-Agent decisions are recorded.
- Fix-Agent handled blocker/request-changes findings.
- Commit-Agent committed only scoped, commit-ready changes.

## Gate L. Playable-game scenario integrity

Required for tasks `0017` through `0028` and changes to gameplay, match flow, gameplay protocol, TCP/WebSocket integration, Qt, Telegram, or playable docs:

- The task states which playable scenario it advances from `docs/project/PLAYABLE_GAME_AGENT_PLAN.md`.
- Two-player Objective Run remains testable and server-authoritative.
- 21x13 map assumptions and 180-degree symmetry are preserved or an ADR is added.
- Player-oriented client views do not change canonical server coordinates.
- README/run docs do not claim playability without a tested or manually verified path.
