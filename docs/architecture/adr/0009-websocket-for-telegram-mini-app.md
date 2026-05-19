# ADR 0009: WebSocket Transport for Telegram Mini App

## Status

Accepted.

## Context

Telegram Mini App runs in a WebView/browser environment. Browser clients do not open arbitrary raw TCP sockets. They can use WebSocket for bidirectional communication.

## Decision

Telegram Mini App connects to the backend through WebSocket over WSS in public deployment.

Raw TCP remains available for Qt/CLI/load clients.

## Consequences

Positive:

- Telegram client can run without installing desktop app;
- WebSocket is browser-compatible;
- same backend can support multiple UIs.

Negative:

- another transport must be tested;
- WebSocket backpressure/slow-client behavior must be handled explicitly;
- WSS/TLS deployment is required for public demo.

## Security rules

- Limit WebSocket message size.
- Validate every message through shared protocol parser.
- Enforce rate limits.
- Bound outgoing queues.
- Use WSS publicly.
