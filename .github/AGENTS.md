# GitHub and Workflow Agent Rules

This directory is protected.

## Rules

- Do not edit `.github/workflows/**` unless the task packet explicitly allows CI/CD changes.
- Any workflow change requires Gate I from `docs/review/QUALITY_GATES.md`.
- Keep workflow permissions minimal. Default to `contents: read`.
- PR CI must not deploy and must not require production secrets.
- Main CI may package artifacts only after build and structure checks pass.
- Do not add third-party actions without reviewing trust, pinning strategy, and permissions.
- Do not expose secrets in logs.

Read also: `docs/ci/CI_CD_GUARDRAILS.md` and `.agents/skills/cmake-ci/SKILL.md`.

## Secret handling

- Secret scanning must remain enabled in PR CI and main CI.
- PR CI must not require production secrets or deploy credentials.
- Workflow edits that remove `scan_secrets.py` require explicit Security Review Agent approval.
- Do not add workflow steps that echo environment variables or print token-bearing payloads.
