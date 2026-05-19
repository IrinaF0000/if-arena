# Implementation Plan

This plan is written so that coding agents can take one task at a time. Every task must use `docs/agent-seeds/TASK_PACKET_TEMPLATE.md`.

## Global implementation rules

- Do not modify the original repository.
- Copy the original local simulation into `external/battle_simulation_snapshot/` only as a read-only reference.
- Implement production code outside `external/`.
- Keep `battle_core` free from TCP, WebSocket, Qt, Telegram, deployment, and process-level server code.
- Do not implement a feature by weakening security, removing tests, or bypassing architecture boundaries.
- Prefer vertical slices over broad rewrites.
- Keep LLM token use low: read only relevant files, summarize findings, and avoid dumping large file contents into prompts.

## Phase 0. Repository bootstrap

Goal: make the skeleton usable.

Tasks:

1. Verify CMake placeholder build.
2. Verify TypeScript Mini App placeholder install/build.
3. Add `CONTRIBUTING.md` with quality gates.
4. Confirm `external/battle_simulation_snapshot/` is empty and documented.
5. Confirm PR CI covers CMake configure/build/test, Telegram Mini App typecheck/lint/build, and repository structure checks.
6. Confirm main CI repeats important checks on the merged tree and checks packaging inputs without building Docker until the server binary target is ready.
7. Confirm `.github/workflows/**` is treated as protected and governed by `docs/ci/CI_CD_GUARDRAILS.md`.

Acceptance:

- Clean clone can configure/build C++ placeholders.
- Frontend placeholder can run lint/build after dependencies are installed.
- Docs explain how to add the old project snapshot.

## Phase 1. Original project import and core extraction

Goal: extract reusable core without touching old repository.

Tasks:

1. Copy old project into `external/battle_simulation_snapshot/`.
2. Inventory old modules:
   - simulation state;
   - systems;
   - actions;
   - replay;
   - config loading;
   - rendering/tools;
   - CLI/app-specific code.
3. Create extraction map from old paths to new `src/battle_core/` paths.
4. Move/copy only game logic into `src/battle_core/`.
5. Create `BattleEngine` facade.
6. Add deterministic reference scenario test.
7. Add snapshot/event DTOs needed by server.
8. Keep offline runner or replay tool separate from `battle_core`.

Acceptance:

- `battle_core` builds as a static library.
- Reference scenario output matches old snapshot behavior or differences are documented.
- `battle_core` has no dependencies on networking, Qt, Telegram, or process server code.
- Core extraction checklist is complete.

Required reviews:

- Architecture-Agent.
- Verification-Agent.

## Phase 2. Protocol foundation

Goal: define shared message model independent of TCP/WebSocket.

Tasks:

1. Add protocol envelope.
2. Add protocol version.
3. Add message DTOs.
4. Add strict parser/serializer.
5. Add protocol limits.
6. Add validation for:
   - required fields;
   - field types;
   - string length;
   - enum values;
   - command sequence;
   - message size.
7. Add tests for valid and invalid payloads.
8. Update `docs/architecture/PROTOCOL.md`.

Acceptance:

- Protocol is transport-independent.
- Negative tests cover malformed JSON, unknown type, missing fields, oversized fields, invalid command.
- No parser exception crosses the transport boundary unhandled.

Required reviews:

- Security-Agent.
- Architecture-Agent.

## Phase 3. Backend application layer

Goal: implement one authoritative backend independent of transport.

Tasks:

1. Add `SessionId`, `ConnectionId`, `PlayerId`, `MatchId`.
2. Add `ClientSession` abstraction independent of TCP/WebSocket.
3. Add `MatchManager`.
4. Add `MatchWorker`.
5. Add command queue.
6. Add tick loop.
7. Add command validation.
8. Add snapshot/event fanout.
9. Add rate limiter.
10. Add bounded outgoing queue policy.
11. Add metrics counters and timers.

Acceptance:

- Backend can run a match without real sockets using fake sessions.
- Server validates command ownership.
- Match state is owned by one worker.
- Slow session simulation cannot grow memory without bound.
- Metrics are emitted in logs or a simple endpoint.

Required reviews:

- Architecture-Agent.
- Security-Agent.
- Performance-Agent.

## Phase 4. Raw TCP transport

Goal: support Qt/CLI/load clients over raw TCP.

Tasks:

1. Implement async accept/read/write.
2. Implement length-prefixed framing.
3. Handle partial frames.
4. Handle multiple frames in one read.
5. Reject oversized frames before large allocation.
6. Connect transport session to `battle_backend`.
7. Add ping/pong and idle timeout.
8. Add disconnect reasons.
9. Add integration tests with fake TCP clients.
10. Update deployment docs for raw TCP port.

Acceptance:

- TCP transport passes hostile-input tests.
- TCP client can create/join a match through backend.
- No TCP code leaks into `battle_core`.
- Slow client handling is tested.

Required reviews:

- Security-Agent.
- Performance-Agent.

## Phase 5. CLI and load clients

Goal: provide debugging and performance tools.

Tasks:

1. CLI client can connect, authenticate as local demo user, create/join match, and send commands.
2. Load client can simulate configurable clients.
3. Load client reports:
   - successful connections;
   - failed connections;
   - commands/s;
   - p50/p95/p99 latency;
   - disconnect reasons.
4. Load scenarios include:
   - normal clients;
   - slow readers;
   - command spam;
   - invalid payloads.

Acceptance:

- Load client produces report under `reports/load/`.
- Performance doc explains test environment and honest results.
- Server remains stable under tested load.

Required reviews:

- Performance-Agent.
- Security-Agent for hostile scenarios.

## Phase 6. Qt desktop client

Goal: implement desktop client over raw TCP.

Tasks:

1. Add Qt dependency setup.
2. Implement `NetworkClient` separate from UI widgets.
3. Implement connection panel.
4. Implement lobby/create/join flow.
5. Implement arena rendering with `QGraphicsView` or equivalent.
6. Implement keyboard/mouse controls.
7. Implement HUD: HP, cooldowns, score, connection state.
8. Implement event log.
9. Handle disconnect and errors gracefully.
10. Add manual UI checklist.

Acceptance:

- Qt client plays a local match against another client.
- Qt client does not link to `battle_core`.
- No blocking network calls on UI thread.
- UI clearly distinguishes team, enemy, neutral hazards, selected object, and target.

Required reviews:

- UI/UX-Agent if available.
- Architecture-Agent for separation.
- Verification-Agent.

## Phase 7. WebSocket gateway

Goal: support browser/WebView clients over WebSocket.

Tasks:

1. Implement WebSocket endpoint using the same backend.
2. Add WebSocket message size limits.
3. Add handshake timeout.
4. Add ping/pong or keepalive.
5. Validate messages with `battle_protocol`.
6. Add WebSocket-specific disconnect reasons.
7. Add tests for malformed, oversized, spam, and slow clients.
8. Update `docs/telegram/TELEGRAM_SECURITY.md`.

Acceptance:

- WebSocket client can create/join/play the same match mode.
- No WebSocket code leaks into `battle_core`.
- WebSocket gateway enforces bounded resources.
- Security tests pass.

Required reviews:

- Security-Agent.
- Performance-Agent.

## Phase 8. Telegram Mini App

Goal: implement Telegram UI using TypeScript and WebSocket.

Tasks:

1. Add strict TypeScript/Vite project.
2. Add Telegram bridge module.
3. Read `window.Telegram.WebApp.initData` and send raw `initData` to backend.
4. Never trust `initDataUnsafe` for authority.
5. Add WebSocket client with reconnect state machine.
6. Add runtime validation for inbound messages.
7. Add Canvas arena renderer.
8. Add touch controls:
   - virtual joystick or D-pad;
   - attack;
   - dash;
   - interact.
9. Add desktop fallback controls.
10. Add lobby/join by code.
11. Add responsive layout.
12. Add build and lint scripts.

Acceptance:

- Mini App connects to backend through WebSocket.
- Backend validates Telegram init data before creating an authenticated Telegram session.
- Frontend has strict TypeScript and no unchecked `any`.
- Mini App can play the MVP match.
- Frontend contains no secrets.

Required reviews:

- Frontend-Agent.
- Security-Agent.
- UX-Agent if available.

## Phase 9. Security hardening

Goal: make public demo safer.

Tasks:

1. Complete threat model.
2. Add fuzz/negative tests for protocol where practical.
3. Enforce all resource limits through config.
4. Add public server config with conservative defaults.
5. Add log redaction for tokens and raw init data.
6. Add dependency review.
7. Add secure deployment guide.
8. Add abuse scenarios:
   - oversized frames;
   - JSON bombs;
   - command spam;
   - reconnect flood;
   - slow read clients;
   - invalid Telegram init data;
   - replayed auth data.

Acceptance:

- All security requirements have test or documented mitigation.
- Public demo config is conservative.
- Security-Agent signs off.

## Phase 10. Observability and load testing

Goal: show high-load-oriented engineering.

Tasks:

1. Implement periodic metrics logging.
2. Add structured load report format.
3. Add load test documentation.
4. Add sample result file.
5. Add profiling notes template.
6. Add tuning knobs:
   - tick rate;
   - max clients;
   - max matches;
   - queue sizes;
   - rate limits;
   - snapshot frequency.

Acceptance:

- Load test can be reproduced locally.
- Report includes environment and limits.
- README includes honest numbers.

## Phase 11. Deployment

Goal: deploy server and frontend for demo.

Tasks:

1. Dockerize `battle_server_app`.
2. Document raw TCP and WebSocket ports.
3. Document TLS termination for WebSocket.
4. Document Telegram Mini App hosting.
5. Document environment variables.
6. Document public firewall rules.
7. Add Oracle Cloud/Railway style examples where applicable.

Acceptance:

- A technical reviewer can understand how to run a remote demo.
- Secrets are not committed.
- Public endpoint uses WSS for Telegram client.

## Phase 12. Portfolio polish

Goal: make project reviewable and CV-ready.

Tasks:

1. Add screenshots/GIFs.
2. Add architecture diagram.
3. Add demo instructions.
4. Add "What this demonstrates" section.
5. Add load test results.
6. Add security notes.
7. Add known limitations.
8. Add CV bullets.

Acceptance:

- README communicates technical value in under 2 minutes.
- Docs are coherent.
- CI passes.


## CI/CD preservation policy for all phases

- Feature agents must not edit `.github/workflows/**` as an incidental fix.
- Any workflow update must be a separate CI/CD task packet.
- PR CI must stay validation-only and secret-free.
- Main CI must stay separate from PR CI.
- Deployment automation must not be added to feature tasks.
- Required checks must be updated in `docs/ci/CI_CD_GUARDRAILS.md` if job names change.
