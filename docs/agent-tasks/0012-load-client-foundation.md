# Task: Create load client foundation

## Phase

Phase 5/10. CLI and load clients; Observability and load testing

## Goal

Add configurable simulated clients and initial load report format.

## Allowed files

- `tools/load_client/`
- `docs/performance/PERFORMANCE_AND_LOAD_TESTING.md`
- `reports/load/`
- `src/battle_protocol/` if client messages are needed

## Forbidden files

- `src/battle_core/`
- `frontend/`

## Required quality gates

- Gate A
- Gate G
- Gate B if protocol sending/parsing changes

## Security impact

medium

## Performance/scalability impact

high

## Architecture impact

medium

## Token budget instructions

- Read load testing docs, load_client files, protocol message shape.

## Implementation steps

1. Add config options for clients, duration, endpoint, command rate.
2. Simulate connection/auth/join loop placeholder.
3. Add latency measurement structure.
4. Add report writer.
5. Add sample report template.

## Required tests/checks

- Load client builds.
- Dry-run mode writes report.
- No unbounded client buffers.

## Required docs updates

- Update only if behavior or structure changes.

## Required reviews

- Performance-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Scope is not broadened.
- Required checks pass or blockers are documented.
- Agent progress is updated with files changed, tests run, and remaining risks.
