# Observability Rules

## Required metrics

- active connections;
- authenticated sessions;
- active matches;
- commands per second;
- invalid commands;
- rejected/rate-limited commands;
- disconnect reasons;
- pending queue sizes;
- tick duration;
- average, p95, p99 command latency;
- bytes in/out.

Metrics must not expose secrets or raw user payloads.
