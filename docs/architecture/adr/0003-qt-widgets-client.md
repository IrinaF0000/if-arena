# ADR 0003: Use Qt Widgets for Desktop Client

## Status

Accepted.

## Context

The project needs a visual client to make the multiplayer demo easy to understand. Options considered:

- Web UI
- Qt Widgets
- Qt Quick/QML
- CLI only

## Decision

Use Qt Widgets with `QGraphicsView`/`QGraphicsScene`.

## Rationale

Qt Widgets is a good fit because:

- it is C++-centric
- it avoids introducing web/frontend stack
- it can use `QTcpSocket`
- it is enough for a simple 2D arena
- it is faster to implement than a polished QML UI
- it demonstrates desktop C++ skills in addition to networking

## Consequences

### Positive

- visible and interactive demo
- keyboard/mouse support
- simple rendering of units, HP bars, and hazards
- no browser or web server required

### Negative

- Qt dependency increases build setup complexity
- CI for Qt can be more involved
- UI will not run directly in a browser

## Notes

The server must not depend on Qt. Qt is only used in the client.
