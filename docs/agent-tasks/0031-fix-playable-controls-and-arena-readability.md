# Task 0031: Fix playable controls and arena readability

## Task title
`Fix playable controls and arena readability`

## Phase
`Post-v0.1.0 playable MVP bugfix/polish`

## Goal
Make the current Qt playable path stable and readable enough for manual play: predictable controls, no live-match freeze, clear objective/combat visuals, and a focused bottom-to-top arena view.

## Background
Manual play after `v0.1.0-playable-mvp` found these issues:

- two Qt clients can join a match, but the live tick can freeze around `1200` while input acknowledgements continue;
- keyboard/mouse input can feel erratic or stop affecting player movement;
- mouse movement appears to affect gameplay unexpectedly;
- idle clients can spam `input accepted (none)`;
- the arena is visually too wide for a bottom-to-top objective game;
- the normal play view looks too much like a debug grid;
- the objective, attack range, attack direction, carrier state, and contest pressure are not readable enough;
- unclear colored sticks/bars above players should be removed or replaced with meaningful indicators.

## Allowed files
- `src/battle_qt_client/**`
- `src/battle_server_app/**` if required to fix the live tick/snapshot loop
- `src/battle_backend/**` if required to fix match tick/application after tick 1200
- `src/battle_core/**` only for a verified gameplay bugfix needed by this task
- `tests/**`
- `docs/game/**`
- `tests/manual/**`
- `README.md` only if user-facing run/play instructions change
- `docs/agent-runs/`

## Forbidden files
- `.github/workflows/**` unless a failing local check proves a minimal docs-only update is required
- `deploy/**`
- `external/**`
- unrelated frontend/Telegram work
- new gameplay modes, classes, progression, or AI systems

## Required quality gates
- Gate A
- Gate C for client authority and input validation
- Gate D for Qt client work
- Gate F/G if load/security behavior is touched
- Gate K: sequential agent pipeline
- Gate L: playable-game scenario integrity

## Sequential agent pipeline
Use `docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md`.
Implementation-Agent must not commit.

## Implementation steps
1. Reproduce or inspect the live tick freeze around tick `1200`; fix the root cause so live matches keep ticking and applying input until match end or disconnect.
2. Define and implement Qt controls from `docs/game/CONTROLS_AND_UI.md`:
   - keyboard moves;
   - mouse aims only;
   - left click/Space attacks;
   - right click/Shift dashes;
   - objective pickup/capture remains automatic.
3. Stop idle no-op input spam. Do not send or log repeated `none` commands while idle.
4. Ensure focus is usable after join: gameplay keys must work without clicking into a text field.
5. Improve arena readability without adding new gameplay systems:
   - reduce strong debug-grid appearance;
   - make the play field feel bottom-to-top rather than unnecessarily wide;
   - keep static objects visually aligned to the logical grid;
   - keep player movement smooth;
   - make objective, carrier, bases, attack range/direction, hit, and dash feedback readable;
   - remove unclear player sticks/bars or replace them with documented indicators.
6. Add a compact controls hint to the Qt UI.
7. Update only the canonical docs that changed behavior relies on; avoid duplicating this task packet.

## Required tests
Use `docs/agent-rules/quality/TESTING.md`; record matrix.

Minimum required coverage:

- live match can tick beyond `1200`;
- movement/input submitted after tick `1200` can still change authoritative state;
- mouse movement alone does not create movement, dash, attack, teleport, or gameplay-command spam;
- keyboard movement still maps to movement intent;
- left click or Space maps to attack intent;
- right click or Shift maps to dash intent;
- arena transform/readability changes do not break existing Qt transform tests;
- existing TCP, load, security, and protocol tests still pass.

If a Qt UI behavior cannot be unit-tested cleanly, add or update the manual checklist and explain why in the Test Impact Matrix.

## Required docs updates
- Keep long-lived controls in `docs/game/CONTROLS_AND_UI.md`.
- Keep long-lived visual rules in `docs/game/VISUAL_READABILITY.md` and `docs/game/MAP_AND_FAIRNESS.md` if needed.
- Keep objective rules in `docs/game/OBJECTIVE_RULES.md` if implementation confirms or changes automatic pickup/capture semantics.
- Update `README.md` only if player-facing play instructions change.

## Manual Qt checklist
Record results in the run note:

1. Start server with `config/examples/server.local.json`.
2. Start two Qt clients.
3. Client 1 connects and creates a match.
4. Client 2 connects and joins by match code.
5. Move mouse over the arena without pressing buttons; player does not move, dash, attack, teleport, or spam inputs.
6. Move with WASD/arrows; player movement is predictable.
7. Left click and Space attack toward aim direction and show feedback.
8. Right click and Shift dash toward aim or movement direction and show feedback.
9. Objective is visually centered/readable when spawned or dropped.
10. Carrier state is obvious.
11. Attack range/direction is visible enough to understand hits and misses.
12. Match remains responsive after 60 seconds; tick continues beyond `1200`.
13. Idle clients do not spam `input accepted (none)`.

## Acceptance criteria
- Two Qt clients can create/join and continue playing after tick `1200`.
- Keyboard movement remains responsive after 60 seconds.
- Mouse movement alone never moves or triggers the player.
- Attack and dash controls are implemented or explicitly disabled with clear UI/docs; no ambiguous behavior remains.
- Idle no-op input spam is removed.
- Objective pickup/capture is automatic and readable, or any intentional deviation is documented and tested.
- Strong debug-grid appearance is removed or made subtle in normal play.
- Arena view is focused on bottom-to-top objective play and not unnecessarily stretched horizontally.
- Unclear colored player sticks/bars are removed or replaced with meaningful documented indicators.
- Required automated tests and manual checklist are recorded in `docs/agent-runs/0031-fix-playable-controls-and-arena-readability.md`.

## Required reviews
- Coordinator: yes
- Architecture-Agent: yes if backend/core tick/input flow changes
- Security-Agent: yes if protocol validation, authority, or rate limits change
- Performance-Agent: yes if input rate, render loop, or snapshot loop changes
- Qt-Agent: yes
- Verification-Agent: yes

## Rollback note
One scoped commit. Merge to `master` with a no-ff merge only after checks and manual Qt checklist pass. This task must not create a release tag.
