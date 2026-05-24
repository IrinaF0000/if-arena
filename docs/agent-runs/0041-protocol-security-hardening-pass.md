# Task 0041: Protocol and security hardening pass

## State

committed

## Coordinator Preflight

- Branch: `agent/0041-protocol-security-hardening-pass`.
- Baseline: `master` after local merge of task 0040.
- Working tree at preflight had one untracked local file not owned by this task: `IF_Arena_Codex_Roadmap_After_Public_0033.md`. It will remain unstaged unless explicitly requested.
- Task packet created: `docs/agent-tasks/0041-protocol-security-hardening-pass.md`.
- Allowed implementation scope confirmed: protocol unit tests, TCP security negative smoke, task docs.
- Forbidden scope confirmed: no protocol schema behavior changes unless a new test exposes a real defect, no workflows, deploy files, release tags, push, or gameplay/frontend feature work.
- Required sequence: Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent -> Commit-Agent -> Coordinator closeout.

## Implementation-Agent Prompt

Implement only task 0041. Add focused hardening coverage for missing/unsupported protocol version, wrong payload type, bad sessionSeq, command-before-match, authority fields, and no raw sensitive payload logging through the TCP negative smoke. Preserve current protocol/server semantics unless a new test exposes a real defect.

## Implementation-Agent

- Added protocol unit coverage for unsupported protocol version, wrong payload type, and negative `sessionSeq`.
- Expanded the TCP security negative smoke to cover missing version, wrong payload type, bad `sessionSeq`, command before match, and existing authority-field rejection in one server run.
- Added an explicit sensitive sentinel payload and assertion that server output does not include the raw hostile payload.
- Kept protocol/server behavior unchanged because the new tests passed against existing rejection semantics.

## Test Impact Matrix

Changed behavior:

- No production behavior changed; this task strengthens protocol/security regression coverage.

Tests added/updated:

- Happy path: existing `tcp_vertical_slice_smoke.py` and CTest suite remain passing.
- Corner cases: protocol unit tests now cover unsupported version, non-object payload, and negative `sessionSeq`.
- Invalid input / hostile input: TCP security smoke now covers missing version, wrong payload type, bad `sessionSeq`, command before match, and forbidden authority fields.
- Authority / ownership: authority-field rejection remains covered end-to-end through TCP and backend ownership tests remain passing through CTest.
- Resource bounds / performance: existing CTest and TCP vertical smoke continue covering oversized frames, queue bounds, rate limits, and slow-client paths; this task did not change limits.
- Regression: TCP security smoke now asserts the server does not print the sensitive sentinel payload.
- Manual UI checks: none required; protocol/security boundary coverage is automated.

Not tested and why:

- Telegram auth replay protection is documented as future work and was not implemented in this test-only hardening task.
- GitHub-hosted CI was not run because no push was performed.

Ready for verification: yes.

## Verification-Agent

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON` - passed.
- `cmake --build build --parallel` - passed.
- `ctest --test-dir build --output-on-failure` - passed, 7/7 tests.
- `python tests/security/tcp_protocol_negative.py` - passed.
- `python tests/integration/server/tcp_vertical_slice_smoke.py` - passed.
- `python scripts/ci/validate_architecture_boundaries.py` - passed.
- `python scripts/ci/scan_secrets.py` - passed.
- `python scripts/agent/validate_agent_harness.py` - passed.
- `python scripts/agent/validate_playable_task_sequence.py` - passed.
- `git diff --check` - passed.
- `scripts/ci/validate_structure.sh` through Git Bash shim - passed.

## Review-Agent

- Security Review-Agent: approved. The task adds hostile-input and no-raw-output coverage without weakening auth/protocol behavior or logging raw payloads.
- Architecture Review-Agent: approved. Changes are limited to tests and task docs; no boundary or runtime dependency changed.
- Verification-Agent: approved. Required checks passed.

## Fix-Agent

- Fixed the TCP security smoke after the readiness probe consumed one accepted client slot by setting the test server accepted-client count to include that probe.

## Commit-Agent

- Commit created with message `test: extend protocol security negatives`.
- Final commit hash is recorded in the coordinator response because amending this file changes the hash.
- Scope checked before commit: only task 0041 files were staged; root roadmap file remained unstaged.

## Coordinator Closeout

- Task 0041 is complete on branch `agent/0041-protocol-security-hardening-pass`.
- Final commit hash is recorded in the coordinator response.
- Ready for local merge to `master`.
- No push or release tag was performed.
