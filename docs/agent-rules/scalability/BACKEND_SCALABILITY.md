# Backend Scalability Rules

## Required design principles

- Async I/O for network transports.
- Bounded queues for sessions and backend commands.
- Backpressure policy for slow clients.
- Rate limiting for command spam.
- Match isolation through worker/actor-like ownership.
- Avoid shared mutable game state across worker threads.
- Measure tick duration, queue sizes, active clients, active matches, bytes in/out, command latency.

## Resource bounds

Every public input path must have explicit limits:

- frame/message size;
- pending write bytes;
- input queue length;
- commands per second;
- clients per server;
- matches per server;
- players per match.
