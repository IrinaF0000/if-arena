# IF Arena Coordination Plan

Date: 2026-05-19

Coordinator scope: planning only. Do not implement code from this plan. Assign one task packet per agent run and keep changes inside the packet's allowed files.

## Source Notes

- Requested process files `docs/agent-rules/process/TASK_PACKETS.md` and `docs/agent-rules/process/WORKTREE_AND_BRANCHING.md` are not present.
- Current equivalents read for this plan: `docs/agent-rules/process/TASK_PACKET_RULES.md` and `docs/agent-rules/process/WORKTREE_AND_BRANCHES.md`.
- `docs/AGENTS.md` is the nearest instruction file for this run note.

## Coordination Rules

- Keep one branch/worktree per task packet.
- Merge sequential dependencies before dependent work.
- Do not let two agents edit the same public DTO/API concurrently.
- Do not edit `.github/workflows/**`, `deploy/**`, or source code from coordination-only work.
- If CI/CD files are needed, use the dedicated `0013` packet.
- If a task crosses a forbidden dependency direction in `PROJECT_MAP.md`, stop and request Architecture Review Agent before continuing.
- Treat network, Telegram, config, replay, and load-test input as untrusted.
- Keep progress notes short: changed files, checks, decisions, risks, next steps.

## Sequential Tasks

This is the recommended merge order for implementation packets.

| Order | Packet | Worktree / branch name | Primary dependency | Required review routing |
| --- | --- | --- | --- | --- |
| 1 | `0001-bootstrap-build.md` | `codex/0001-bootstrap-build` | none | Verification |
| 2 | `0002-import-old-project-snapshot.md` | `codex/0002-import-old-project-snapshot` | `0001` | Coordinator, Verification |
| 3 | `0003-core-extraction-audit.md` | `codex/0003-core-extraction-audit` | `0002` | Architecture, Verification |
| 4 | `0004-battle-core-facade.md` | `codex/0004-battle-core-facade` | `0003` | Architecture, Verification |
| 5 | `0005-protocol-envelope-and-limits.md` | `codex/0005-protocol-envelope-limits` | `0001`, coordinate with `0004` DTO needs | Security, Architecture, Verification |
| 6 | `0006-backend-session-model.md` | `codex/0006-backend-session-model` | `0004`, `0005` | Security, Architecture, Performance, Verification |
| 7 | `0007-tcp-transport-framing.md` | `codex/0007-tcp-framing` | `0005`, preferably `0006` | Security, Performance, Verification |
| 8 | `0008-qt-client-network-skeleton.md` | `codex/0008-qt-network-skeleton` | `0005`, `0007` protocol/framing shape | Architecture, Verification, Qt review if available |
| 9 | `0009-websocket-transport-skeleton.md` | `codex/0009-ws-transport-skeleton` | `0005`, `0006` | Security, Architecture, Performance, Verification |
| 10 | `0010-telegram-mini-app-skeleton.md` | `codex/0010-telegram-mini-app-skeleton` | `0005`, `0009` message shape | Security for Telegram bridge, Frontend, Verification |
| 11 | `0011-telegram-auth-validation.md` | `codex/0011-telegram-auth-validation` | `0006`, `0009`, `0010` auth request shape | Security, Architecture, Verification |
| 12 | `0012-load-client-foundation.md` | `codex/0012-load-client-foundation` | `0005`, `0007`, later `0009` | Performance, Verification; Security if hostile scenarios/protocol change |

Maintenance packets:

| Packet | Worktree / branch name | When to run | Required review routing |
| --- | --- | --- | --- |
| `0013-ci-cd-guardrails.md` | `codex/0013-ci-cd-guardrails` | Only for dedicated CI/CD changes | Coordinator, Security, Verification, CI/CD review |
| `0014-agent-harness-scoped-instructions.md` | `codex/0014-agent-harness-scoped-instructions` | When task packets, rules, or skills need scoped cleanup | Agent Manager; Security if security rules move/weaken; CI/CD if scripts/workflows change |
| `0015-secrets-and-safe-logging-guardrails.md` | `codex/0015-secrets-safe-logging` | Before or alongside auth/logging work | Security |

## Parallelizable Work

- After `0001`, `0002` and `0005` may start in parallel if protocol work stays in `src/battle_protocol/`, protocol tests, and protocol docs. Merge carefully if `0005` needs snapshot/event shapes from `0004`.
- After `0002`, `0003` is sequential with the snapshot import and must not start until the snapshot exists.
- `0015` can run alongside early product work if it avoids overlap with active CI/script edits and does not touch real secrets.
- After `0005` and `0006` stabilize, `0007` and `0009` can run in parallel if they do not edit the same transport tests or protocol docs without coordination.
- `0008` can begin as a skeleton after protocol shape is stable, but it should not invent authoritative client rules.
- `0010` can begin after WebSocket message assumptions are documented, but Telegram auth authority must remain backend-owned.
- `0012` can begin with TCP dry-run/report scaffolding after `0007`; WebSocket load scenarios wait for `0009`.
- `0013` must remain isolated from feature branches.
- `0014` should be a maintenance branch and should not run concurrently with another agent editing the same agent-rule files.

## Dependency Graph

```text
0001-bootstrap-build
  -> 0002-import-old-project-snapshot
      -> 0003-core-extraction-audit
          -> 0004-battle-core-facade
              -> 0006-backend-session-model
  -> 0005-protocol-envelope-and-limits
      -> 0006-backend-session-model
      -> 0007-tcp-transport-framing
      -> 0009-websocket-transport-skeleton
      -> 0010-telegram-mini-app-skeleton
      -> 0012-load-client-foundation

0006-backend-session-model
  -> 0007-tcp-transport-framing
  -> 0009-websocket-transport-skeleton
  -> 0011-telegram-auth-validation

0007-tcp-transport-framing
  -> 0008-qt-client-network-skeleton
  -> 0012-load-client-foundation

0009-websocket-transport-skeleton
  -> 0010-telegram-mini-app-skeleton
  -> 0011-telegram-auth-validation
  -> 0012-load-client-foundation

0010-telegram-mini-app-skeleton
  -> 0011-telegram-auth-validation

0013-ci-cd-guardrails
  -> independent dedicated branch only

0014-agent-harness-scoped-instructions
  -> independent maintenance branch only

0015-secrets-and-safe-logging-guardrails
  -> recommended before 0011 and any logging/auth work
```

## First Implementation Wave

Wave 1 should establish build confidence, import/audit the original snapshot, and start protocol/security foundations without broadening scope.

### Packet W1-A: Bootstrap Placeholder Build

- Base packet: `docs/agent-tasks/0001-bootstrap-build.md`.
- Goal: verify C++ placeholder configure/build and frontend placeholder script consistency.
- Non-goals: no gameplay implementation, no snapshot import.
- Allowed files: `CMakeLists.txt`, `frontend/telegram_mini_app/`, `docs/project/REPOSITORY_BOOTSTRAP.md`.
- Coordination override: do not edit `.github/workflows/**` in this wave. If workflow changes are required, stop and open `0013`.
- Required checks: CMake configure/build; confirm frontend `typecheck` and `lint` scripts exist.
- Definition of done: required checks pass or blockers documented; progress note written.
- Reviews: Verification Agent.

### Packet W1-B: Import Original Project Snapshot

- Base packet: `docs/agent-tasks/0002-import-old-project-snapshot.md`.
- Goal: copy the old local simulation into `external/battle_simulation_snapshot/` as read-only reference.
- Non-goals: no refactor, no production source changes.
- Allowed files: `external/battle_simulation_snapshot/`, `docs/project/ORIGINAL_PROJECT_IMPORT.md`, `docs/project/CORE_EXTRACTION_CHECKLIST.md`.
- Required checks: no build artifacts/binaries committed; snapshot README/source note exists.
- Definition of done: snapshot import documented with source/date if available.
- Reviews: Coordinator, Verification Agent.

### Packet W1-C: Core Extraction Audit

- Base packet: `docs/agent-tasks/0003-core-extraction-audit.md`.
- Goal: create concise extraction map from old simulation files to future `battle_core` modules.
- Non-goals: no `src/` implementation changes.
- Allowed files: `external/battle_simulation_snapshot/`, `docs/project/CORE_EXTRACTION_CHECKLIST.md`, `docs/project/ORIGINAL_PROJECT_IMPORT.md`, `docs/agent-notes/`.
- Required checks: documentation-only review; token economy followed.
- Definition of done: old files categorized as core/app/tools/ignore with risks called out.
- Reviews: Architecture Agent, Verification Agent.

### Packet W1-D: Protocol Envelope And Limits

- Base packet: `docs/agent-tasks/0005-protocol-envelope-and-limits.md`.
- Goal: define transport-independent envelope, limits, parser/serializer, and negative tests.
- Non-goals: no client implementation, no `battle_core` changes.
- Allowed files: `src/battle_protocol/`, `tests/unit/protocol/`, `docs/architecture/PROTOCOL.md`.
- Required checks: valid envelope parses; malformed JSON, unknown type, oversized string, and missing fields are rejected.
- Definition of done: parser returns explicit error results and no parser exception crosses transport boundary.
- Reviews: Security Agent, Architecture Agent, Verification Agent.

### Packet W1-E: Secrets And Safe Logging Guardrails

- Base packet: `docs/agent-tasks/0015-secrets-and-safe-logging-guardrails.md`.
- Goal: keep secret placeholders, redaction expectations, and secret scanning ready before Telegram/auth work.
- Non-goals: no real secrets, no unrelated gameplay/transport/frontend changes.
- Allowed files: packet-listed secret/logging guardrail files only.
- Required checks: `python3 scripts/ci/scan_secrets.py`; `bash scripts/ci/validate_structure.sh`.
- Definition of done: example env files contain placeholders only and future auth/logging tasks have clear guardrails.
- Reviews: Security Agent.

## Review Agent Routing

Security Review Agent required:

- `0005-protocol-envelope-and-limits.md`
- `0006-backend-session-model.md`
- `0007-tcp-transport-framing.md`
- `0009-websocket-transport-skeleton.md`
- `0010-telegram-mini-app-skeleton.md` for Telegram bridge/auth/network surface
- `0011-telegram-auth-validation.md`
- `0013-ci-cd-guardrails.md`
- `0015-secrets-and-safe-logging-guardrails.md`
- `0012-load-client-foundation.md` if it adds hostile payload scenarios or changes protocol parsing/sending

Architecture Review Agent required:

- `0003-core-extraction-audit.md`
- `0004-battle-core-facade.md`
- `0005-protocol-envelope-and-limits.md`
- `0006-backend-session-model.md`
- `0008-qt-client-network-skeleton.md`
- `0009-websocket-transport-skeleton.md`
- `0011-telegram-auth-validation.md`
- Any task that changes public DTO/API boundaries or crosses a forbidden dependency direction

Performance Review Agent required:

- `0006-backend-session-model.md`
- `0007-tcp-transport-framing.md`
- `0009-websocket-transport-skeleton.md`
- `0012-load-client-foundation.md`
- Any task that changes queues, workers, broadcasts, tick loop, metrics, or load scenarios

## Risks And Blockers

- Original project snapshot source is not identified in this repository; `0002` needs the local source path from the user or coordinator.
- Requested process-rule filenames differ from current repo filenames; run `0014` later if naming consistency should be fixed.
- `0001` packet currently lists a workflow file as allowed, but this coordination request forbids workflow edits; route any CI change to `0013`.
- Protocol and core facade can influence each other through snapshot/event DTOs; coordinate before merging `0004` and `0005`.
- Frontend and Telegram auth must not treat `initDataUnsafe` or client state as authoritative.
