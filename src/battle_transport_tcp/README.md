# battle_transport_tcp

Raw TCP transport adapter.

Owns:
- async accept/read/write;
- length-prefixed framing;
- partial/combined frame handling;
- TCP connection lifecycle;
- transport metrics labels.

Must forward parsed protocol messages to battle_backend.
Must not implement gameplay rules.
