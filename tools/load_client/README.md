# battle_load_client

Synthetic load generator for the TCP backend.

## Purpose

This tool simulates many clients to demonstrate backend behavior under load.

It is part of the portfolio story. It should produce reproducible reports, not just ad-hoc traffic.

## Example usage

```text
battle_load_client --server 127.0.0.1 --port 5555 --clients 100 --matches 50 --ramp-up 10 --duration 180 --scenario gameplay --output reports/load_100.json
```

Current foundation usage:

```text
battle_load_client --dry-run --endpoint 127.0.0.1:4000 --clients 100 --duration 60 --command-rate 10 --scenario connect_only --output reports/load/dry-run-report.md
```

Dry-run mode does not open sockets. It validates bounded options and writes the initial markdown report format.

## Required scenarios

- `connect_only`
- `create_and_join`
- `gameplay`
- `slow_readers`
- `spam_clients`
- `malformed_clients`

## Required metrics

- clients started;
- clients connected;
- connection failures;
- commands sent;
- acknowledgements received;
- average latency;
- p95 latency;
- p99 latency;
- disconnects;
- protocol errors.

## Rules

- Use the same `battle_protocol` codec as real clients.
- Make tests reproducible by supporting a random seed.
- Do not hide failures. Report them clearly.
- Do not use unrealistic client behavior unless the scenario explicitly says so.
- Keep generated client-side buffers bounded by configuration.
