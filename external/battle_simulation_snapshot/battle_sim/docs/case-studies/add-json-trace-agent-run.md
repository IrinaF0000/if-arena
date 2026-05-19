# Case Study: Add JSON Trace Output

## Context

The project already had deterministic legacy stdout events. The task added a second optional machine-readable trace without replacing legacy output.

## Task Packet

Goal: add `--trace-json <file>` and keep stdout unchanged.

Allowed files: App CLI/runner, IO trace writer, event DTOs, tests, README/docs.

Forbidden changes: no external JSON dependency, no UI, no legacy output rewrite.

Acceptance criteria: valid deterministic JSON trace, golden test coverage, documented usage.

## Agents Involved

- Lead agent: implementation, integration, and verification.
- Review pass: checked behavior compatibility, ownership boundaries, and test coverage.

## Implementation Summary

- Added `JsonTraceWriter` as an optional event sink.
- Added a composite event sink in the App layer.
- Added CLI support for `--trace-json`.
- Extended spawn events with archetype id so traces can include `unitType`.

## Verification Output

```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python scripts/check_architecture_boundaries.py
python -m json.tool build/trace-test.json
```

All checks passed.

## Review Findings

- Keep legacy stdout as the compatibility surface.
- Keep trace writing deterministic and small.
- Avoid a JSON dependency until the schema grows.

## Final Patch Notes

The trace is useful for inspection tools and future regression checks. It remains optional and does not affect existing command files.

## Lessons Learned

Event mirroring belongs in App composition. Feature events should carry enough data for IO adapters instead of forcing adapters to query world state.
