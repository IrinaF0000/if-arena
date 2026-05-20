# Foundation Through 0015 Release Integration

Date: 2026-05-20

## Safety Branch

- `agent/foundation-through-0015-integration`
- Safety snapshot commit: `9398278` (`Safety snapshot foundation through 0015`)
- Split plan commit on safety branch: `2f8747a` (`Record foundation split plan`)

The safety snapshot was not merged. It only preserved the original uncommitted implementation state from dirty `master`.

## Branches And Commits

| Merge order | Branch | Focused commit |
| --- | --- | --- |
| 1 | `agent/core-protocol-foundation` | `52ea47e` Add core and protocol foundations |
| 2 | `agent/backend-session-foundation` | `845a7c4` Add backend session foundation |
| 3 | `agent/transports-foundation` | `d118d07` Add transport foundations |
| 4 | `agent/clients-foundation` | `d0926a7` Add client foundations |
| 5 | `agent/load-observability-foundation` | `8724ca1` Add load observability foundation |
| 6 | `agent/security-secrets-foundation` | `c8163c4` Add security and secrets foundations |
| 7 | `agent/agent-harness-foundation` | final reporting/harness commit on this branch |

## Merges Completed Before This Report

- `631cd09` Merge core and protocol foundations
- `300511b` Merge backend session foundation
- `ce3037a` Merge transport foundations
- `5251ac0` Merge client foundations
- `73bf9c7` Merge load observability foundation
- `a317e18` Merge security and secrets foundations

## Checks Per Branch

### `agent/core-protocol-foundation`

- CMake configure/build with `-DBATTLE_BUILD_QT_CLIENT=OFF` passed.
- `ctest` passed: 2 tests.
- `python scripts/ci/scan_secrets.py` passed.
- `python scripts/agent/validate_agent_harness.py` passed.
- `git diff --check` passed.
- Post-merge master checks passed with the same test set.

### `agent/backend-session-foundation`

- CMake configure/build passed.
- `ctest` passed: 3 tests.
- `python scripts/ci/scan_secrets.py` passed.
- `python scripts/agent/validate_agent_harness.py` passed.
- `git diff --check` passed.
- Post-merge master checks passed with the same test set.

### `agent/transports-foundation`

- CMake configure/build passed.
- `ctest` passed: 5 tests.
- `python scripts/ci/scan_secrets.py` passed.
- `python scripts/agent/validate_agent_harness.py` passed.
- `git diff --check` passed.
- Post-merge master checks passed with the same test set.

### `agent/clients-foundation`

- CMake configure/build with `-DBATTLE_BUILD_QT_CLIENT=OFF` passed.
- Extra CMake configure/build with `-DBATTLE_BUILD_QT_CLIENT=ON` passed.
- `ctest` passed: 5 tests.
- `npm.cmd run typecheck` passed.
- `npm.cmd run lint` passed.
- `npm.cmd run build` passed after rerunning outside the sandbox because the initial Vite/esbuild config resolution hit an access-denied error.
- `python scripts/ci/scan_secrets.py` passed.
- `python scripts/agent/validate_agent_harness.py` passed.
- `git diff --check` passed.
- Post-merge master checks passed, including frontend typecheck/lint/build.

### `agent/load-observability-foundation`

- CMake configure/build passed.
- `ctest` passed: 5 tests.
- Load-client smoke check passed after rerunning with approval to write a temporary dry-run report.
- Temporary generated report `reports/load/split-dry-run.md` was removed and not committed.
- `python scripts/ci/scan_secrets.py` passed.
- `python scripts/agent/validate_agent_harness.py` passed.
- `git diff --check` passed.
- Post-merge master checks passed with the same test set.

### `agent/security-secrets-foundation`

- CMake configure/build passed.
- `ctest` passed: 6 tests.
- `python scripts/ci/scan_secrets.py` passed.
- `gitleaks dir . --config .gitleaks.toml --redact --no-banner` passed.
- `python scripts/agent/validate_agent_harness.py` passed.
- `git diff --check` passed.
- Post-merge master checks passed with the same test set and gitleaks.

### `agent/agent-harness-foundation`

- Pending at this report draft: final harness/reporting checks must pass before merge.

## Environment Blockers And Residual Risks

- `bash scripts/ci/validate_structure.sh` could not be run because `bash` is not available in this Windows PowerShell environment.
- PyYAML is not installed. No `.github/workflows/**`, `deploy/**`, or `docs/ci/**` changes were part of this split, so YAML validation was not required for a CI/YAML slice.
- `npm.cmd install` reported two moderate audit findings in frontend dependencies. Dependency upgrades were not included in this release integration split.
- The first `npm.cmd install` attempt timed out under the default sandbox; the approved rerun completed.
- `npm.ps1` is blocked by PowerShell execution policy, so frontend checks used `npm.cmd`.

## Final Git Status

Expected after the final harness branch is committed, merged, and checked:

- current branch: `master`
- working tree: clean
- ignored generated directories may remain locally, including CMake build trees, `frontend/telegram_mini_app/node_modules/`, and `frontend/telegram_mini_app/dist/`
