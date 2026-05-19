# Agent Rules Template

These rules apply to all coding agents working on IF Arena.

## Mission

Build a high-quality C++20 real-time authoritative arena backend with two clients:

- Qt desktop client over raw TCP;
- Telegram Mini App over WebSocket.

Quality, security, scalability, architecture boundaries, and tests are more important than speed.

## Hard boundaries

### Original project snapshot

- `external/battle_simulation_snapshot/` is read-only reference.
- Do not add TCP, WebSocket, Qt, Telegram, server, load-test, or deployment code under `external/`.
- Do not modify the original repository outside this project.
- Extract reusable code into `src/battle_core/`.


### CI/CD and workflow preservation

- `.github/workflows/**`, `.github/actions/**`, `.github/CODEOWNERS`, `scripts/ci/**`, `deploy/**`, and `docs/ci/**` are protected CI/CD-sensitive paths.
- Do not edit CI/CD-sensitive paths unless the task packet explicitly allows CI/CD changes.
- Feature implementation tasks must not weaken, disable, or bypass CI checks.
- Pull request workflows must not deploy and must not use production secrets.
- Main-branch workflows must preserve least-privilege permissions and protected deployment behavior.
- Workflow changes require Coordinator, Security-Agent, and Verification-Agent review.
- Read `docs/ci/CI_CD_GUARDRAILS.md` before changing CI/CD-sensitive files.

### Module boundaries

- `battle_core` must not depend on TCP, WebSocket, Qt, Telegram, deployment, server app, metrics exporter, or process-level code.
- `battle_protocol` must remain transport-independent.
- `battle_backend` owns authority, sessions, matches, validation orchestration, rate limiting, backpressure, and metrics.
- `battle_transport_tcp` owns raw TCP only.
- `battle_transport_ws` owns WebSocket only.
- `battle_qt_client` must not link to `battle_core`.
- `frontend/telegram_mini_app` must not contain server secrets or authoritative rules.

## Security rules

- Treat all network input as hostile.
- Validate all protocol messages.
- Reject oversized messages before unbounded allocation.
- Never assume one TCP read equals one protocol message.
- Never trust client-reported position, HP, score, cooldowns, team, match result, or player id.
- Validate Telegram Mini App raw `initData` on the backend.
- Never trust `initDataUnsafe`.
- Never log raw init data, tokens, session secrets, or full hostile payloads by default.
- Enforce rate limits and bounded queues.
- Add negative tests for security-relevant code.

## Scalability rules

- Do not create one thread per client.
- Do not add unbounded queues, maps, buffers, or logs.
- Every growing resource must have a configured limit.
- Slow clients must not cause unbounded memory growth.
- Match state should be owned by a single worker.
- Add metrics when adding a queue, worker, transport, or rate limit.

## C++ rules

- Use C++20.
- Use RAII.
- No C-style casts.
- No raw owning pointers in new code.
- No hidden mutable globals.
- Avoid shared mutable state.
- Make ownership explicit.
- Prefer small cohesive classes.
- Keep blocking operations away from I/O threads and UI threads.
- Do not swallow exceptions silently.
- Prefer deterministic tests.

## Qt rules

- No blocking socket operations in UI thread.
- Keep networking in `NetworkClient` or equivalent.
- UI widgets must not own protocol parsing complexity.
- UI widgets must not own game rules.
- Use clear signal/slot boundaries.
- Keep rendering separate from connection/session state.

## TypeScript rules

- Strict TypeScript.
- No unchecked `any`.
- Treat WebSocket input as `unknown`.
- Validate before use.
- No secrets in frontend.
- Do not trust Telegram frontend identity.
- No raw `innerHTML` from untrusted values.
- Keep modules small.
- Do not paste generated files or dependency directories into prompts.

## LLM token economy rules

Agents must conserve LLM tokens:

- Read only files needed for the task.
- Prefer `rg`, file lists, and focused snippets over dumping whole files.
- Summarize findings before editing.
- Do not paste large generated files, lockfiles, build logs, or full dependency trees.
- Do not re-read unchanged files repeatedly.
- Keep progress updates short.
- Update only task-relevant docs.
- Prefer small patches over broad rewrites.
- If context is large, create a short local summary file and cite it in the progress note.
- Never include `node_modules`, build directories, compiled binaries, generated assets, or large traces in prompts.

## Completion rule

A task is not complete until:

- relevant code builds or the reason is explicitly documented;
- relevant tests are added or updated;
- security impact is assessed;
- performance/resource impact is assessed;
- docs are updated if behavior/API/architecture changed;
- required quality gate is satisfied;
- progress file is updated with exact files changed.
