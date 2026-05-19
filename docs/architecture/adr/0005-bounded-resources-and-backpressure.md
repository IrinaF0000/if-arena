# ADR 0005: Enforce bounded resources and backpressure

## Status

Accepted

## Context

TCP servers can be harmed by slow readers, command spam, oversized frames, and unbounded queues. A portfolio project exposed publicly should demonstrate defensive resource management.

## Decision

All network-facing and hot-path queues must have explicit limits. The server shall implement backpressure behavior for slow clients and rate limiting for command spam.

Required limits include:

- max clients;
- max matches;
- max frame bytes;
- max pending write bytes per client;
- max incoming queue size per client;
- max match command queue size;
- max commands per second per client.

## Consequences

Benefits:

- prevents unbounded memory growth;
- makes overload behavior explicit;
- improves security posture;
- supports credible load-test reports.

Costs:

- clients can be disconnected under pressure;
- snapshots may need coalescing;
- tests must cover overload behavior.
