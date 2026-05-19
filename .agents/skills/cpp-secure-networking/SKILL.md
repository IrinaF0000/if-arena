# Skill: C++ Secure Networking

Use when changing TCP, WebSocket, framing, parsing, session I/O, or network-adjacent C++ code.

## Checklist

- Treat input as untrusted.
- Enforce max sizes before allocation.
- Handle partial reads/writes and EOF.
- Use bounded queues.
- Avoid blocking event loops.
- Prefer explicit state machines over implicit flags.
- Use RAII for sockets/resources.
- Return structured errors at boundaries.
- Add negative tests for malformed input.

## Output expectation

Report changed files, security decisions, and negative tests added. Do not paste full code.
