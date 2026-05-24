# Reconnect And Resume

## Current MVP

IF Arena does not yet support secure match resume after a connection closes. A Web or Telegram client that disconnects while in a match must treat the old backend session as gone, clear local match state, and show that rejoin/resume is not supported yet.

The client may reconnect to the WebSocket and authenticate again, but it must not send commands for the previous match until the server accepts a fresh `join_match` flow.

## Target Semantics

Local browser resume can use a short-lived reconnect token or client identity stored in browser storage only after Security Review approves the token format, expiry, and binding rules.

Telegram resume must use a backend-validated Telegram user id from signed raw `initData`. The frontend must never treat `initDataUnsafe`, a stored user id, or a locally generated token as authoritative identity.

The server-side resume flow should:

- reserve a disconnected player slot for a short grace period;
- bind the resume token to the validated identity, match, player slot, and expiry;
- allow only the same identity to resume the same player;
- reject wrong identity, expired token, wrong match, and replayed token attempts;
- avoid logging raw tokens, Telegram init data, or signed payloads.

## Non-Goals For MVP

- No frontend-only resume token.
- No long-lived localStorage secret.
- No protocol schema change.
- No backend slot reservation.
- No automatic reconnect loop.
