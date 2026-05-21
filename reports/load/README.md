# Load Reports

Load client reports are written here.

Generated reports should include the scenario, client count, duration, command rate, latency percentiles, disconnects, observed bottlenecks, and follow-up notes.

Current report sources:

- `battle_load_client --dry-run --scenario <name> --output reports/load/<file>.md`
- `python tests/load/local_tcp_load_scenarios.py --report reports/load/0028-local-tcp-smoke.md`

Dry-run reports validate bounded scenario planning only. Local TCP smoke reports use real sockets but are intentionally tiny and must not be described as production capacity.
