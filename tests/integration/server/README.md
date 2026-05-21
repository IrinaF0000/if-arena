# server

Server and client integration tests.

## Raw TCP vertical slice

After building the C++ targets, run:

```bash
python tests/integration/server/tcp_vertical_slice_smoke.py
```

The smoke starts a temporary local TCP server config on an ephemeral port and checks:

- two TCP clients can authenticate, create/join, receive snapshots, and submit an intention command;
- malformed JSON and unknown message types receive structured protocol errors;
- oversized frame lengths close before payload allocation;
- missing authentication hits the handshake timeout.
