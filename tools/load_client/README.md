# battle_load_client

Synthetic load generator for the TCP backend.

## Purpose

This tool simulates many clients to demonstrate backend behavior under load.

It is part of the portfolio story. It should produce reproducible reports, not just ad-hoc traffic.

## Example usage

```text
battle_load_client --dry-run --endpoint 127.0.0.1:5555 --clients 100 --duration 60 --command-rate 10 --scenario gameplay --seed 42 --output reports/load/dry-run-report.md
```

Dry-run mode does not open sockets. It validates bounded options and writes the markdown report format for a named scenario. Live local raw TCP smoke lives in `tests/load/local_tcp_load_scenarios.py`.

## Required scenarios

- `connect_only`
- `create_and_join`
- `gameplay`
- `slow_readers`
- `command_spam`
- `invalid_payloads`

## Required metrics

- clients started;
- clients connected;
- connection failures;
- commands sent;
- acknowledgements received;
- p50 latency;
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
- Do not present dry-run counts as throughput.
