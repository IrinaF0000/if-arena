# C++ Quality Rules

Target: modern C++20, clear ownership, portable builds, testable modules.

## Required

- RAII for resources.
- Strongly typed IDs for players, sessions, matches, units where practical.
- `std::unique_ptr` for ownership when dynamic allocation is required.
- `std::shared_ptr` only for shared lifetime that is explicitly justified.
- `std::chrono` for time.
- `std::span`, `std::string_view`, or typed buffers where appropriate.
- Explicit error handling at network and parsing boundaries.
- Small cohesive classes.

## Forbidden by default

- Raw owning pointers.
- C-style casts.
- Hidden global mutable state.
- Unbounded queues or buffers.
- Catch-all exception swallowing.
- Blocking calls inside event loops or UI thread.
- Undefined behavior, data races, or lifetime tricks.

## Tests

Any non-trivial C++ change must include or update tests unless the task packet explicitly marks it as docs-only or skeleton-only.
