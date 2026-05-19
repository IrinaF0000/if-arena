# Access Boundaries

## Default stance

No agent owns the whole repository. Every implementation task must define allowed and forbidden files.

## Protected areas

These paths require explicit task permission:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- `docs/ci/**`
- `external/battle_simulation_snapshot/**`
- security-sensitive auth/session code
- public protocol schemas used by multiple transports

## Stop conditions

Stop and report instead of editing when:

- the task needs a forbidden file;
- architecture constraints conflict;
- security requirements are unclear;
- a change would require rewriting another agent's work;
- secrets or real credentials appear in files.
