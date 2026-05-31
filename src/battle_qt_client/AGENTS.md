# Qt Client Agent Rules

The Qt client is a desktop visual client to implement Qt/C++ skills and raw TCP networking.

## Responsibilities

- Qt Widgets UI.
- Keyboard/mouse controls.
- Rendering server snapshots.
- Sending player intentions to the backend through the TCP client layer.
- Connection status and user-friendly errors.

## Forbidden behavior

- Do not implement authoritative game rules in the client.
- Do not mutate game state locally except for visual interpolation/prediction explicitly approved by architecture docs.
- Do not block the UI thread on network or file I/O.
- Do not store secrets in UI code.

Read also: `.agents/skills/qt-client/SKILL.md` and `docs/agent-rules/quality/QT.md`.
