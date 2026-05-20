# 0014 Agent Harness Scoped Instructions

Date: 2026-05-19

## Summary

- Reviewed root `AGENTS.md`, `docs/agent-rules/README.md`, `docs/agent-manager/AGENT_MANAGER_ROLE.md`, and the harness validator.
- Kept root `AGENTS.md` unchanged and short.
- Hardened `scripts/agent/validate_agent_harness.py` so the existing process, security, quality, and scalability rule files are required by validation.

## Changed files

- `scripts/agent/validate_agent_harness.py` - added existing core rule files to the required harness file list.
- `docs/agent-runs/0014-agent-harness-scoped-instructions.md` - recorded task progress.

## Checks run

- `python scripts\agent\validate_agent_harness.py` - passed.
- `bash scripts/ci/validate_structure.sh` - passed using Git Bash with a local `python3` shim.

## Risks and follow-up

- Agent Manager review is required.
- No security rules were weakened or moved.
- No CI workflow behavior was changed.
