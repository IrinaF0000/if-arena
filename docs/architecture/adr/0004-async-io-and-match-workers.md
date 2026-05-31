# ADR 0004: Use async I/O and match workers

## Status

Accepted

## Context

The project should implement high-load backend practices. A thread-per-client model is simple but does not show scalable server design and can waste resources under many connections.

The simulation also needs a clear threading model to avoid data races.

## Decision

Use asynchronous TCP I/O for client sessions. Use simulation workers where each match is owned by a single worker. Networking code forwards validated commands to the owning match worker.

## Consequences

Benefits:

- avoids one thread per client;
- keeps match state ownership simple;
- reduces locking around gameplay state;
- supports load testing with many simulated clients;
- makes tick duration measurable per worker.

Costs:

- more complex lifecycle management;
- requires careful queue and shutdown handling;
- requires explicit metrics and tests for overload cases.
