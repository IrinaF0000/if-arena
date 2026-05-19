# Observability

## Goal

Make the backend understandable during local demo, load tests, and public experiments.

## MVP observability

Initial implementation may use structured periodic logs:

```text
[metrics] transport=tcp clients=100 matches=50 cmds_s=1800 tick_p95_ms=4.2 out_queue_p95_kb=12 disconnects_slow=0
```

Later, add HTTP `/metrics` or Prometheus exporter.

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

Log at error level:

- transport failure affecting listener;
- match worker crash;
- config load failure.

## Agent requirement

Any task adding a queue, worker, transport, or rate limiter must update observability docs or explain why no new metric is needed.
