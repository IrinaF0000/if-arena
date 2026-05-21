# battle_cli_client

Small command-line client for debugging, integration tests, and protocol experiments.

Responsibilities:

- connect to server by host/port;
- send hello/create_match/join_match/command messages;
- print snapshots and events;
- support scripted inputs for test scenarios.

## Current TCP mode

By default the CLI opens a raw TCP connection, authenticates with local demo auth, creates or joins a match, and sends scripted player intentions. It prints server envelopes as they arrive. The client still owns no authoritative state.

Build and run the default local flow:

```bash
cmake --build build --parallel
build/battle_server_app --config config/examples/server.local.json --max-clients 2
```

In two other terminals:

```bash
build/battle_cli_client --create --display-name cli-one --script tests/integration/server/cli_idle.script
build/battle_cli_client --join M1 --display-name cli-two --script tests/integration/server/cli_scenario_b.script
```

Fake-connect mode remains available for offline protocol transcript checks:

```bash
build/battle_cli_client --fake-connect --create --match-id local-match --script tests/integration/server/cli_scenario_b.script
```

Script lines are one command per line:

```text
move DX DY
attack DX DY
dash DX DY
interact
stop
wait MS
```

Directions are bounded to `-1..1`. The generated protocol payloads contain intentions only: command kind, direction when required, match id, and `sessionSeq`.
