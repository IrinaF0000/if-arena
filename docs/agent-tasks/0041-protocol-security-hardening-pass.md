# Task 0041: Protocol and security hardening pass

## Goal

Audit and strengthen client-server boundary coverage after the playable browser/server path became more complete.

## Scope

Allowed files:

- `tests/unit/protocol/ProtocolEnvelopeTests.cpp`
- `tests/security/tcp_protocol_negative.py`
- `docs/agent-runs/0041-protocol-security-hardening-pass.md`
- `docs/agent-tasks/0041-protocol-security-hardening-pass.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- protocol schema behavior changes unless a new negative test exposes a real defect
- `src/battle_core/**`
- frontend gameplay/UI work
- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- release tags
- pushing to GitHub

## Required behavior

- Add focused negative coverage for missing version, unsupported version, wrong payload type, bad `sessionSeq`, command before match, and client authority fields.
- Preserve existing rejection semantics and structured error behavior.
- Assert the server does not print raw sensitive payload sent by a hostile client in the TCP negative smoke.
- Do not weaken or remove existing tests.

## Required quality gates

- Gate A. Every implementation task
- Gate F. Security-sensitive protocol/network boundary coverage
- Gate K. Sequential agent pipeline
- Gate M. Architecture boundary enforcement

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `python tests/security/tcp_protocol_negative.py`
- `python tests/integration/server/tcp_vertical_slice_smoke.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Security-Agent: yes
- Architecture-Agent: yes
- Verification-Agent: yes

## Acceptance criteria

- Negative protocol unit tests cover the roadmap parser/validator gaps.
- TCP security smoke rejects invalid client boundary cases without raw payload leakage in server output.
- Existing backend/transport security tests remain passing.
- No production secrets, protocol schema changes, or deployment changes are introduced.

## Rollback note

Revert this task commit to remove the added hardening coverage.
