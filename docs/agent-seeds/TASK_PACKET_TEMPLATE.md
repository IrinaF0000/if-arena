# Task Packet Template

## Task title

`<short imperative title>`

## Phase

`<phase number and name from IMPLEMENTATION_PLAN.md>`

## Goal

`<one clear outcome>`

## Background

`<brief context, not a full project recap>`

## Allowed files

- `<exact paths or directories>`

## Forbidden files

- `external/battle_simulation_snapshot/` unless task is only copying/import inspection.
- Any unrelated module.
- Build outputs.
- Generated dependency directories.
- Secret/config files with real credentials.
- `.github/workflows/**` unless this is a dedicated CI/CD task.
- `scripts/ci/**`, `deploy/**`, and `docs/ci/**` unless explicitly allowed.

## Required quality gates

- Gate A: every implementation task.
- Gate B: networking/protocol if applicable.
- Gate C: backend/gameplay if applicable.
- Gate D: Qt if applicable.
- Gate E: Telegram Mini App if applicable.
- Gate F: security-sensitive if applicable.
- Gate G: performance/load if applicable.
- Gate H: documentation-only if applicable.
- Gate I: CI/CD and workflow changes if applicable.


## CI/CD impact

`none / low / medium / high`

Explain:

- Does this touch `.github/workflows/**`, `.github/actions/**`, `scripts/ci/**`, `deploy/**`, or `docs/ci/**`?
- Does this change PR checks, main checks, required checks, artifacts, permissions, secrets, or deployment behavior?
- Does this require `docs/ci/CI_CD_GUARDRAILS.md` update?

## Security impact

`none / low / medium / high`

Explain:

- What trust boundary is touched?
- What hostile input exists?
- What secrets or identity data exist?
- What negative tests are required?

## Secrets/logging impact

`none / low / medium / high`

Explain:

- Does this touch environment variables, auth, logging, deployment, CI secrets, Telegram validation, or token handling?
- Does this require Gate J and Security Review Agent approval?
- Which secret-related tests or scans are required?

## Performance/scalability impact

`none / low / medium / high`

Explain:

- Does this add a queue, buffer, map, thread, timer, worker, or broadcast path?
- What bounds are required?
- What metrics are required?

## Architecture impact

`none / low / medium / high`

Explain:

- Does this change dependencies?
- Does it touch public APIs?
- Does it affect core/backend/transport/client boundaries?

## Token budget instructions

- Read only:
  - `<file 1>`
  - `<file 2>`
- Use `rg` for:
  - `<symbols/terms>`
- Do not read:
  - `node_modules/`
  - `build/`
  - generated assets
  - full old snapshot unless explicitly required
- Keep final progress note under `<N>` lines.

## Implementation steps

1. `<small step>`
2. `<small step>`
3. `<small step>`

## Required tests

- `<test 1>`
- `<test 2>`

## Required docs updates

- `<doc path or "none">`

## Acceptance criteria

- `<criterion 1>`
- `<criterion 2>`
- `<criterion 3>`

## Required reviews

- Coordinator: yes
- Architecture-Agent: yes/no
- Security-Agent: yes/no
- CI/CD-Agent or Coordinator workflow review: yes/no
- Performance-Agent: yes/no
- Frontend-Agent: yes/no
- Qt-Agent: yes/no
- Verification-Agent: yes

## Rollback note

`<how to revert or isolate the change>`


## Secret handling checklist

- Does this task touch environment variables, auth, logging, deployment, CI, or Telegram validation? yes/no
- If yes, Gate J and Security Review Agent are required.
- Real secrets must not be added. Update `.env.example` only with placeholders.
- Run `python3 scripts/ci/scan_secrets.py` before reporting completion.
