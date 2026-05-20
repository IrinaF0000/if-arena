# IF Arena Load Client Report

## Config

- mode: dry_run
- endpoint: 127.0.0.1:4000
- scenario: connect_only
- clients: 5
- durationSeconds: 3
- commandRatePerSecond: 2

## Results

- clientsStarted: 5
- clientsConnected: 5
- commandsPlanned: 30
- disconnects: 0
- p50LatencyMs: 0
- p95LatencyMs: 0
- p99LatencyMs: 0

## Notes

- Dry-run mode does not open sockets.
- Live transport, auth/join loop, and latency sampling are future work.
