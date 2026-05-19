# Scripts Agent Rules

Scripts are shared automation and may be invoked by CI or agents.

## Rules

- Prefer portable Bash or Python 3.
- Scripts must fail fast and print actionable errors.
- Do not rely on secrets for validation scripts.
- Do not modify protected files unless explicitly designed and reviewed for that task.
- Agent helper scripts must be advisory unless task packets or CI explicitly require them.
