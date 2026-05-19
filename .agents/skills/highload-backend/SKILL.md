# Skill: High-load Backend

Use for match workers, queues, rate limits, backpressure, metrics, load testing, or tick loop work.

## Checklist

- Define resource bounds.
- Avoid unbounded memory growth.
- Isolate match state per worker/actor.
- Track queue sizes and latency percentiles.
- Make slow-client behavior explicit.
- Use deterministic clocks/seeds in tests.
- Add load-test scenarios and report format.
