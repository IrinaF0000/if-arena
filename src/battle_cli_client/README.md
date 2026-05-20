# battle_cli_client

Small command-line client for debugging, integration tests, and protocol experiments.

Responsibilities:

- connect to server by host/port;
- send hello/create_match/join_match/command messages;
- print snapshots and events;
- support scripted inputs for test scenarios.

## Current fake-connect mode

Real TCP wiring is intentionally deferred to task 0025. The CLI currently supports a clearly labeled fake-connect mode that validates and prints protocol intentions without opening a socket or mutating client-owned game state.

Build and run the default Scenario B intention script:

```bash
cmake --build build --target battle_cli_client --parallel
build/battle_cli_client --fake-connect --create --match-id local-match
```

Run the checked-in script file:

```bash
build/battle_cli_client --fake-connect --create --match-id local-match --script tests/integration/server/cli_scenario_b.script
```

Join-style transcript:

```bash
build/battle_cli_client --fake-connect --join LOCAL1 --match-id local-match --display-name cli-two
```

Script lines are one command per line:

```text
move DX DY
attack DX DY
dash DX DY
interact
stop
```

Directions are bounded to `-1..1`. The generated protocol payloads contain intentions only: command kind, direction when required, match id, and `sessionSeq`.
