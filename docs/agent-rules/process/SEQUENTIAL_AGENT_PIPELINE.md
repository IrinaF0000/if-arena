# Sequential Agent Pipeline

Mandatory orchestration for every non-trivial IF Arena task. Task packets must reference this file instead of duplicating the workflow.

## Mandatory order

```text
Coordinator preflight -> Implementation-Agent -> Verification-Agent -> Required Review-Agent(s) -> Fix-Agent if needed -> Verification-Agent after fixes -> Commit-Agent -> Coordinator closeout
```

Implementation-Agent must not commit. Review-Agent must not silently edit code. Fix-Agent must only fix listed findings. Commit-Agent must refuse to commit if any required gate, review, check, or Test Impact Matrix is missing.

## Task state

Use these states in the run note:

```text
ready -> implementing -> verifying -> reviewing -> fixing -> re-verifying -> commit-ready -> committed
```

Allowed terminal states: `committed`, `blocked`, `reverted`, `superseded`. Stop and split the task if the same blocker survives two fix attempts.

## Role contracts

### Coordinator preflight

- Select one task packet from `docs/agent-tasks/`.
- Confirm working tree status, branch/worktree, allowed files, forbidden files, gates, and required reviews.
- Create or update `docs/agent-runs/<task-id>-<slug>.md`.
- Prepare the exact Implementation-Agent prompt.
- Do not broaden scope.

### Implementation-Agent

- Read the task packet, `PROJECT_MAP.md`, nearest `AGENTS.md`, and only relevant rule files.
- Summarize assumptions before editing.
- Make the smallest scoped change that satisfies the task.
- Add/update tests according to `docs/agent-rules/quality/TESTING.md`.
- Update docs only for changed behavior, API, protocol, architecture, workflow, or run instructions.
- Run focused checks and write an implementation note.

Output:

```text
Changed files:
Checks run:
Test Impact Matrix:
Known risks:
Follow-up needed:
Ready for verification: yes/no
```

### Verification-Agent

- Inspect diff scope and forbidden paths.
- Run required gates and record exact commands/results.
- Reject missing tests, missing Test Impact Matrix, weakened tests, unrelated changes, or unapproved skipped checks.
- Add missing tests only when directly required by the task; otherwise send findings to Fix-Agent.
- Do not redesign the feature.

Minimum checks for C++ implementation tasks:

```bash
cmake -S . -B build -DBATTLE_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
git diff --check
python3 scripts/ci/scan_secrets.py
python3 scripts/agent/validate_agent_harness.py
```

Frontend changes also require:

```bash
cd frontend/telegram_mini_app
npm install
npm run typecheck
npm run lint
npm run build
```

Playable planning/task changes also require:

```bash
python3 scripts/agent/validate_playable_task_sequence.py
```

Playable gameplay scenario changes also require:

```bash
python3 scripts/ci/validate_no_hardcoded_scenarios.py
python3 scripts/ci/validate_gameplay_scenario_pairs.py
```

For playable gameplay work, all game scenarios must be config-driven. Tests must not hardcode scenario routes, map data, hazards, or expected event sequences. Every `tests/scenarios/*.json` gameplay scenario must have both desktop and mobile coverage that can be proven by `validate_gameplay_scenario_pairs.py`.

### Required Review-Agent(s)

Run only the reviewers required by the task packet and quality gates.

- Architecture-Agent: APIs, dependencies, module boundaries, ownership, threading.
- Security-Agent: auth, parser, network input, session identity, logging, config, deployment, hostile input.
- Performance-Agent: tick loop, queues, workers, broadcasting, metrics, load, slow clients.
- Frontend-Agent: Telegram Mini App, WebSocket browser state, TypeScript, runtime validation.
- Qt-Agent: Qt UI, signal/slot boundaries, non-blocking networking, rendering, input.
- CI/CD-Agent: `.github/**`, `scripts/ci/**`, `deploy/**`, `docs/ci/**`.

Review output:

```text
Decision: approve / approve-with-minors / request-changes / block
Findings:
- [severity] file:line or area - issue - required fix
Required re-checks:
```

`request-changes` or `block` requires Fix-Agent before commit.

### Fix-Agent

- Fix only listed verification/review findings.
- Update tests when behavior changes.
- Avoid opportunistic refactoring.
- Record fixed findings, changed files, checks, remaining risks, and readiness for re-verification.

### Verification-Agent after fixes

- Re-run failed and affected checks.
- Confirm no unrelated changes appeared.
- Mark `commit-ready` only when all required gates pass or approved exceptions are documented.

### Commit-Agent

- Confirm state is `commit-ready`.
- Confirm run note, gates, reviews, checks, and Test Impact Matrix are recorded.
- Inspect `git status --short`.
- Refuse commits with forbidden files, generated dependencies, build outputs, mixed scope, failed secret scan, missing reviews, missing tests, or implementation-agent commits.
- Commit only scoped task changes.

Commit format:

```text
<area>: <imperative summary>

- <main change>
- <tests/checks>
- <risk or follow-up if any>
```

### Coordinator closeout

- Record commit hash, final status, known risks, and follow-up tasks.
- Do not start the next task until the current one is terminal.

## Anti-patterns that block completion

- Implementation goes straight to commit.
- Review findings are marked resolved without a fix/re-check.
- Happy-path-only tests for network, parser, auth, session, gameplay, or resource-limit code.
- Missing negative tests for invalid input or authority violations.
- Docs claim a playable scenario that was not tested or manually verified.
- Agent docs duplicate large blocks from canonical rules instead of linking them.
