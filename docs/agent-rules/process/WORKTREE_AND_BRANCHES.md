# Worktrees and Branches

## Recommended pattern

One agent task should use one branch/worktree.

Examples:

- `agent/core-extraction`
- `agent/protocol-envelope`
- `agent/tcp-transport`
- `agent/ws-transport`
- `agent/qt-client`
- `agent/telegram-mini-app`
- `agent/load-testing`

## Merge discipline

- Merge sequential dependencies first.
- Rebase active branches after shared protocol/core changes.
- Do not let two agents edit the same public DTO/API concurrently.
- Keep CI/CD changes in a dedicated branch.
