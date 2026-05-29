# Run 0065: Desktop side panel layout

## Status

State: commit-ready
Branch: `agent/0047-playable-stabilization`
Task packet: `docs/agent-tasks/0065-desktop-side-panel-layout.md`

## Scope

- Qt presentation layout and manual checklist/docs.
- No gameplay rules, protocol/schema, scenario config, CI/CD, deploy, generated output, push, tag, or merge changes.

## Progress

- Coordinator preflight: completed.
- Implementation-Agent: completed.
- Verification-Agent: completed.
- Review-Agent: completed.
- Commit-Agent: pending.

## Assumptions

- Existing Qt signals/network separation remains unchanged; this task only rearranges widgets and base rendering.

## Test Impact Matrix

Changed behavior:
- Qt desktop layout now uses a main arena area with a stable right side panel for connection, match, status, hint/error, next-match, and event-log UI.
- Qt base rendering now uses compact scoring-zone markers instead of full-width bands.
- Qt UI docs/manual checklist describe the side panel and smaller base visuals.

Tests added/updated:
- Happy path: Qt build/CTest and desktop gameplay scenarios still pass.
- Corner cases: resize behavior is supported by a fixed-width side panel and stretch arena layout.
- Invalid input / hostile input: no parser/network input surface changed.
- Authority / ownership: layout and base visuals are presentation-only; clients still send intentions and render authoritative snapshots.
- Resource bounds / performance: event log remains capped at 120 blocks.
- Regression: architecture, agent, secret, and diff checks passed.
- Manual UI checks: checklist updated for right side panel and compact bases.

Not tested and why:
- Live GUI screenshot was not captured in this run; Qt compiled/linked and scenario runners cover the desktop client path, while the manual checklist records visual acceptance steps.

## Checks

- `cmake --build build --parallel` - passed, including `battle_qt_client.exe`.
- `ctest --test-dir build --output-on-failure` - passed, 8/8 tests.
- `python tests/integration/desktop/objective_run_full_capture_desktop.py` - passed.
- `python tests/integration/desktop/objective_event_sequence_desktop.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `git diff --check` - passed with line-ending normalization warnings only.

## Reviews

- Verification-Agent: approved; scoped Qt/docs changes passed required checks.
- Architecture-Agent: approved; UI layout does not change module dependencies or client authority.
- Qt-Agent: approved; service controls/logs are in the side panel and rendering remains snapshot-driven.

## Risks

- Live visual acceptance should still be checked in an interactive Qt window before final push.
