# Backlog

Backlog items are intentionally small and agent-friendly. Every item touching networking, protocol, security, gameplay validation, or threading requires the relevant quality gates.

## Epic A. Original project import and core extraction

- [ ] Copy original project into `external/battle_simulation_snapshot/` as read-only reference.
- [ ] Remove binaries/build outputs from snapshot.
- [ ] Map old project modules to future `battle_core` modules.
- [ ] Extract world/state model into `src/battle_core/`.
- [ ] Extract action/rule execution into `src/battle_core/`.
- [ ] Extract scenario/feature-pack loading into `src/battle_core/`.
- [ ] Add `BattleEngine` facade.
- [ ] Add deterministic reference scenario test.
- [ ] Add snapshot DTO for server output.
- [ ] Add event DTO for replay/debug output.

## Epic B. Arena gameplay

- [ ] Add player ownership model.
- [ ] Add controller concept: remote player, neutral AI/hazard, scripted test client.
- [ ] Add hero unit archetype.
- [ ] Add move command and validation.
- [ ] Add attack command and validation.
- [ ] Add dash/dodge command and cooldown, if included in MVP.
- [ ] Add interact/capture command.
- [ ] Add 21x13 map config and validation.
- [ ] Add 180-degree rotational symmetry validation for map data.
- [ ] Add objective entity and scoring rules.
- [ ] Add carrier speed multiplier.
- [ ] Add objective drop-on-hit.
- [ ] Add pickup lock after objective drop.
- [ ] Add capture/respawn flow.
- [ ] Add tower hazard.
- [ ] Add mine hazard.
- [ ] Add drone/crow hazard.
- [ ] Add server-side snapshot fields for HP, cooldown, score, objective, hazards.
- [ ] Add local in-process playable simulation test.

## Epic C. Protocol

- [ ] Define protocol version and handshake messages.
- [ ] Implement length-prefixed encoder.
- [ ] Implement incremental frame decoder.
- [ ] Add tests for split frames.
- [ ] Add tests for combined frames.
- [ ] Add tests for oversized frames.
- [ ] Add tests for invalid JSON.
- [ ] Add schema validation for each client message.
- [ ] Add ack/error message types.
- [ ] Add ping/pong message types.
- [ ] Update `docs/architecture/PROTOCOL.md` after implementation.

## Epic D. Server

- [ ] Add async TCP acceptor.
- [ ] Add client session state machine.
- [ ] Add bounded incoming queue.
- [ ] Add bounded outgoing queue.
- [ ] Add backpressure policy for snapshots.
- [ ] Add command rate limiter.
- [ ] Add handshake timeout.
- [ ] Add idle timeout.
- [ ] Add lobby manager.
- [ ] Add match manager.
- [ ] Add match worker ownership model.
- [ ] Add fixed tick loop.
- [ ] Add snapshot broadcast.
- [ ] Add graceful shutdown.
- [ ] Add disconnect reason metrics.

## Epic E. Qt client

- [ ] Add connection window.
- [ ] Add lobby window/panel.
- [ ] Add arena scene.
- [ ] Add player-oriented coordinate transform.
- [ ] Render own base at bottom for both players.
- [ ] Add object rendering shapes and colors.
- [ ] Add subtle grid/tile floor rendering.
- [ ] Add HP bars.
- [ ] Add cooldown indicators.
- [ ] Add objective status panel.
- [ ] Add event log.
- [ ] Add keyboard input handling.
- [ ] Add mouse aim/attack handling.
- [ ] Add latency indicator.
- [ ] Add reconnect/error UI.
- [ ] Add screenshot/GIF instructions.

## Epic F. CLI and load clients

- [ ] Add CLI handshake and connect command.
- [ ] Add CLI create/list/join match commands.
- [ ] Add CLI command sender.
- [ ] Add load-client connection ramp-up.
- [ ] Add load-client scripted movement.
- [ ] Add load-client scripted objective interaction.
- [ ] Add load-client latency measurement.
- [ ] Add slow-client test mode.
- [ ] Add spam-client test mode.
- [ ] Add performance report output.

## Epic G. Quality, security, and performance

- [ ] Add protocol unit tests.
- [ ] Add gameplay validation tests.
- [ ] Add objective pickup/drop/pickup-lock/capture tests.
- [ ] Add player-oriented coordinate transform tests.
- [ ] Add map symmetry validation tests.
- [ ] Add integration test with two fake clients.
- [ ] Add malicious frame tests.
- [ ] Add invalid command ownership tests.
- [ ] Add rate-limit tests.
- [ ] Add backpressure tests.
- [ ] Add basic static analysis or compiler warning policy.
- [ ] Add load-test result template.
- [ ] Add profiling notes.

## Epic H. Public demo and portfolio polish

- [ ] Add Dockerfile for server.
- [ ] Add local run scripts.
- [ ] Add public deployment guide.
- [ ] Add README quickstart.
- [ ] Add architecture diagram.
- [ ] Add gameplay GIF/video plan.
- [ ] Add final CV bullet.
- [ ] Add known limitations section.


## CI/CD and workflow safety

- [ ] Configure GitHub branch protection for `main`.
- [ ] Copy `.github/CODEOWNERS.example` to `.github/CODEOWNERS` after replacing placeholders with real owners.
- [ ] Mark `pr-cpp-build`, `pr-telegram-mini-app`, and `pr-docs-and-structure` as required PR checks.
- [ ] Add dependency update strategy for C++ and TypeScript dependencies.
- [ ] Decide whether production-sensitive workflows should pin third-party GitHub Actions by SHA.
- [ ] Add a manual deployment workflow only after deployment security requirements are finalized.
