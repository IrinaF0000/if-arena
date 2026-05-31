# IF Arena Local TCP Load Smoke

## Environment

- dateUtc: 2026-05-21T12:42:55Z
- commit: 7c5d23b
- server: `C:\Users\user\Documents\Projects\if_arena\if_arena\build\battle_server_app.exe`
- scope: local smoke, not production benchmark
- transport: raw TCP length-prefixed JSON
- scenarios: normal create/join/gameplay, duplicate command rejection, malformed payloads

## Results

- clientsStarted: 4
- clientsConnected: 4
- connectionFailures: 0
- commandsSent: 7
- acksReceived: 7
- rejectedCommands: 1
- snapshotsReceived: 8
- malformedPayloadsSent: 2
- protocolErrorsObserved: 2
- disconnectsObserved: 2
- p50AckLatencyMs: 0.33
- p95AckLatencyMs: 0.69
- p99AckLatencyMs: 0.69

## Limits And Honesty

- This run is intentionally tiny and local. It validates scenario wiring and negative handling, not capacity.
- Slow-reader pressure is documented and modeled by `battle_load_client --scenario slow_readers`; a large live slow-reader soak remains future work.
- Public deployment, TLS termination, async accept scaling, and production observability remain out of scope for this local local slice.
