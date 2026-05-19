# Tools Agent Rules

Tools support development, replay, and load testing.

## Rules

- Tools must not weaken production code paths.
- Load tools must identify themselves and respect local default limits unless an explicit override is provided.
- Replay tools must treat trace files as untrusted input.
- Do not add heavy dependencies without a task-level justification.
