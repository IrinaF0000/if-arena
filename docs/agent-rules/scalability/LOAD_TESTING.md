# Load Testing Rules

## Load client requirements

- Simulate many clients without one OS thread per client where possible.
- Support TCP and WebSocket scenarios if feasible.
- Measure latency, throughput, disconnects, errors, memory trends.
- Include slow-reader and spam-client scenarios.
- Respect safe defaults to avoid accidental abuse.

## Reports

Save results under `reports/load/` and summarize scenario, machine, config, client count, match count, duration, latency percentiles, CPU/memory notes, and known limitations.
