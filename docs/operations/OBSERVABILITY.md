# Observability

## Goal

Make the backend understandable during local demo, load tests, and public experiments.

## MVP observability

Initial implementation may use structured periodic logs:

```text
[metrics] transport=tcp clients=100 matches=50 cmds_s=1800 tick_p95_ms=4.2 out_queue_p95_kb=12 disconnects_slow=0
```

Later, add HTTP `/metrics` or Prometheus exporter.

Current local state:

- `battle_server_app` prints startup/config summaries and periodic aggregate metrics.
- Load reports under `reports/load/` capture client-side counts and latency samples for reproducible local smoke runs.
- There is no production metrics endpoint yet.

## Required labels

Use labels consistently:

```text
transport=tcp|websocket
reason=rate_limited|malformed|timeout|slow_client|auth_failed|normal
mode=objective_run
```

## Security

Do not log:

- raw Telegram `initData`;
- auth tokens;
- full hostile payloads;
- session secrets;
- bot token;
- private keys.

## Useful operational events

Log at info level:

- server start and config summary;
- transport started;
- public bind address and port;
- match created;
- match ended;
- aggregate metrics.

Log at warning level:

- malformed input;
- auth failed;
- rate limited;
- slow client disconnected;
- queue limit reached.

Load-test reports should record:

- commit hash or branch under test;
- server config path or inline limits;
- transport;
- scenario;
- clients started/connected;
- commands sent and acknowledgements received;
- rejected commands and protocol errors;
- p50/p95/p99 latency when measured;
- limitations of the run.

Log at error level:

- transport failure affecting listener;
- match worker crash;
- config load failure.

## Agent requirement

Any task adding a queue, worker, transport, or rate limiter must update observability docs or explain why no new metric is needed.
