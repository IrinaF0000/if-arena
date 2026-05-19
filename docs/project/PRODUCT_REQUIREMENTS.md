# Product Requirements

## 1. Product summary

IF Arena is a portfolio-grade C++20 project that demonstrates a real-time authoritative multiplayer backend with two user-facing clients:

1. Qt desktop client over raw TCP.
2. Telegram Mini App over WebSocket/WSS.

Both clients connect to the same backend and use the same server-authoritative game model.

The project starts from a copied snapshot of an existing local deterministic battle simulation. The original repository is not modified. Selected gameplay code is extracted into `battle_core`, then reused by the backend.

## 2. Product goals

### 2.1 Portfolio goals

- Demonstrate modern C++ backend engineering.
- Demonstrate raw TCP socket programming and message framing.
- Demonstrate Qt desktop application development.
- Demonstrate a web/mobile UX through Telegram Mini App.
- Demonstrate transport abstraction: one backend, multiple clients.
- Demonstrate high-load-oriented practices: async I/O, bounded queues, rate limiting, backpressure, metrics, and load tests.
- Demonstrate security-aware design for hostile network input.
- Demonstrate agentic development workflow with strict quality gates and token economy.

### 2.2 Gameplay goals

- Provide a small but understandable 2-player real-time objective arena game.
- Allow fast action through keyboard/mouse in Qt and touch controls in Telegram.
- Use short matches, ideally 2-4 minutes.
- Use Objective Run objective gameplay instead of pure deathmatch.
- Use deterministic server-controlled hazards to make maps interesting.
- Keep MVP rules simple enough to understand in under 30 seconds.

## 3. Non-goals

- No production MMO infrastructure.
- No matchmaking service in MVP.
- No persistent economy, pay-to-win progression, or real-money mechanics.
- No client-side authority over position, HP, cooldowns, score, objective state, or match result.
- No raw TCP access from Telegram Mini App. Browser/WebView clients use WebSocket.
- No DLL/plugin architecture in MVP. `battle_core` is a static library.

## 4. Target users

### 4.1 Player/demo user

A person who wants to quickly try the demo with a friend:

- Qt desktop: run client, enter host/port, join match.
- Telegram: open bot, press Play, invite friend, start match.

### 4.2 Technical reviewer

A recruiter, interviewer, or engineer reviewing the repository:

- Reads architecture docs.
- Sees clean module boundaries.
- Runs tests and load client.
- Inspects secure network handling.
- Sees clear CV-relevant engineering decisions.

### 4.3 Coding agent

An LLM/coding agent implementing a small scoped task:

- Uses `AGENTS.md`, `.agents/skills/`, and focused rule files.
- Uses `docs/agent-tasks/`.
- Follows quality gates, security rules, access boundaries, and token budget rules.

## 5. Functional requirements

### 5.1 MVP game mode: Objective Run

- Two human players spawn on opposite sides of the arena.
- A central objective objective is placed in the exact center.
- Players move, aim, attack, dash/dodge, and interact.
- A player picks up the objective by interacting near it.
- A player scores by carrying the objective into their own base zone.
- The first player to 3 captures wins by default.
- The match timeout is supported.
- The carrier moves slower.
- The carrier drops the objective on successful hit.
- A short pickup lock prevents immediate re-pickup after drop.
- The server updates neutral hazards every tick.

### 5.2 Map and fairness

- MVP map size: 21 x 13 logical cells.
- The map uses 180-degree rotational symmetry around the central objective.
- The whole arena should fit on screen.
- Both players have equal base stats and symmetric start conditions.
- Bases, spawn points, obstacles, hazards, and route risks are symmetric.
- Map layout is grid-authored, but player movement is smooth and real-time.
- Clients render player-oriented views where the local base is at the bottom.
- Server, replay, spectator, and debug views use canonical orientation.

### 5.3 Player controls

Qt desktop client:

- WASD: movement relative to local view.
- Mouse: aim/target.
- Left click or Space: attack.
- Shift: dash/dodge.
- E: interact/pick up/capture.
- Esc: cancel/close menu.

Telegram Mini App:

- Virtual joystick or directional pad.
- Attack button.
- Dash button.
- Interact button.
- Minimal HUD for HP, score, cooldowns, objective state, and connection state.

### 5.4 Client connection

Both clients must support:

- server endpoint configuration;
- connection state display;
- handshake;
- authentication/session setup;
- match creation or join by code;
- graceful disconnect;
- reconnect as stretch goal.

Qt uses raw TCP with length-prefixed JSON frames.

Telegram Mini App uses WebSocket over WSS and passes Telegram Mini App `initData` for server-side validation during login.

### 5.5 Backend

The backend must support:

- one process hosting both TCP and WebSocket transports;
- common session and match management independent of transport;
- transport-independent protocol DTOs;
- match workers;
- tick-based simulation;
- command queues;
- server-side validation;
- snapshots and events;
- metrics;
- resource limits;
- clean shutdown.

### 5.6 Protocol

Protocol must support:

- `hello`;
- `auth_request` / `auth_result`;
- `create_match`;
- `join_match`;
- `match_joined`;
- `input_command`;
- `input_ack`;
- `snapshot`;
- `event_batch`;
- `error`;
- `ping` / `pong`;
- `disconnect_notice`.

Protocol messages must be versioned. Client commands are player intentions only.

### 5.7 Telegram-specific features

MVP:

- Mini App can be launched from a bot button.
- Mini App can send validated `initData` to the backend.
- Mini App supports join by match code or invite parameter.
- Mini App renders the arena with Canvas.
- Mini App uses WebSocket only.
- Gameplay remains server-authoritative.

Stretch:

- Invite link generation.
- Match result summary sent by bot.
- Fullscreen/landscape mode.
- Haptic feedback.

## 6. Non-functional requirements

### 6.1 Security

- Treat every client as hostile.
- Validate Telegram `initData` on the backend before trusting Telegram identity.
- Do not trust `initDataUnsafe`.
- Use WSS/HTTPS for Telegram Mini App deployment.
- Use TLS termination for public WebSocket endpoint.
- Do not log raw tokens, Telegram initData, session secrets, or raw payloads by default.
- Enforce max frame/message sizes.
- Enforce rate limits per session.
- Enforce handshake timeout and idle timeout.
- Reject malformed, oversized, unknown, or out-of-order messages safely.
- Never trust client-reported position, HP, cooldowns, score, team, objective state, match result, or player id.
- Add negative tests for protocol parsing and command validation.

### 6.2 Scalability and performance

- Async I/O for network transports.
- No thread per client.
- Per-session outgoing queue must be bounded.
- Match state is owned by a single match worker.
- Slow clients must not cause unbounded memory growth.
- Server should expose or log metrics:
  - active connections by transport;
  - active matches;
  - commands/s;
  - snapshots/s;
  - bytes in/out;
  - rejected commands;
  - queue sizes;
  - tick duration;
  - p50/p95/p99 command latency;
  - disconnect reasons.
- Load client must simulate many clients and output a reproducible report.

Initial non-binding demo target:

```text
local machine:
  200-1000 simulated clients
  20 server ticks per second
  bounded memory growth
  no unbounded queues
  p95 command latency reported
```

Actual measured results must be documented honestly.

### 6.3 Code quality

C++:

- C++20.
- RAII.
- No raw owning pointers in new code.
- Prefer value types, `std::unique_ptr`, `std::shared_ptr` only for shared lifetime.
- Clear ownership and lifetime documentation.
- No C-style casts.
- No hidden mutable globals.
- No blocking operations on I/O threads.
- No blocking operations on Qt UI thread.
- Separate protocol, transport, backend, core, and UI concerns.
- Tests for business logic and hostile input.

Qt:

- Qt Widgets MVP.
- Use signals/slots clearly.
- Keep `NetworkClient` separate from UI widgets.
- UI must not own game rules.
- No blocking socket calls in the UI thread.
- Clear connection state UX.

TypeScript Mini App:

- TypeScript strict mode.
- No `any` unless justified.
- Small modules.
- Centralized WebSocket client.
- Runtime validation of inbound messages.
- No authoritative game rules.
- No raw Telegram identity trust on the frontend.
- ESLint and Prettier configuration.
- Avoid storing secrets in frontend code.

### 6.4 Maintainability

- Every major module has README.
- Architecture decisions are recorded as ADRs.
- Protocol changes update docs and tests.
- Gameplay rule changes update game docs and tests.
- Agents must work through task packets.
- Changes must be small and reviewable.

## 7. Acceptance criteria for MVP

MVP is complete when:

1. `battle_core` is extracted from copied snapshot as a static library.
2. `battle_server_app` runs one authoritative Objective Run match with deterministic tick loop.
3. Raw TCP transport accepts Qt/CLI clients using length-prefixed JSON.
4. WebSocket transport accepts Telegram Mini App clients.
5. Qt client can join and play a simple match with player-oriented rendering.
6. Telegram Mini App can join and play the same backend mode.
7. The backend validates all commands, hits, objective pickup/drop/capture, and score.
8. Slow/spam clients are handled safely.
9. Load client produces a metrics report.
10. Security and quality gates pass.
11. README includes run instructions and architecture diagram.
12. A short demo GIF/video can be produced.

## 8. References

- Telegram Mini Apps official documentation: https://core.telegram.org/bots/webapps
- WebSocket API overview: https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API
- WebSockets standard: https://websockets.spec.whatwg.org/
