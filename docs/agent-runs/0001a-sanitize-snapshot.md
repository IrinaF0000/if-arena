# 0001a Sanitize Snapshot

Date: 2026-05-19

## Summary

- Found a false positive in replay-viewer JavaScript: `token` was a local DOM button variable, not a secret.
- Reverted the snapshot variable rename and refined `scripts/ci/scan_secrets.py` to ignore local DOM element creation assignments.
- Kept scanner output limited to file, line, and finding type so suspicious values are not printed.

## Checks run

- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `git diff --check` - passed.

## Remaining blockers

- None.
