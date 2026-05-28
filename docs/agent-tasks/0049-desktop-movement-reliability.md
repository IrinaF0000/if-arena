# Task 0049: Desktop movement reliability

## Goal

Make Qt desktop movement reliable when a direction is held, retried after rejection, or stopped, without giving the client authoritative state.

## Scope

Allowed files:

- `CMakeLists.txt`
- `src/battle_qt_client/game/**`
- `src/battle_qt_client/ui/MainWindow.cpp`
- `src/battle_qt_client/ui/MainWindow.hpp`
- `src/battle_qt_client/ui/README.md`
- `src/battle_qt_client/tests/**`
- `tests/scenarios/movement_reliability.json`
- `docs/agent-runs/0049-desktop-movement-reliability.md`
- `docs/agent-tasks/0049-desktop-movement-reliability.md`
- `docs/agent-tasks/README.md`

Forbidden files:

- `.github/workflows/**`
- `scripts/ci/**`
- `deploy/**`
- `docs/ci/**`
- `battle_core` gameplay rules
- backend authority rules
- scenario map redesign
- Telegram Mini App source
- generated build outputs
- generated frontend `dist/**`
- release tags
- pushing to GitHub
- merging into `master` or `main` before final post-0046 acceptance

## Required behavior

- Add a failing config-driven movement reliability test before the fix.
- Keep route/timing expectations in `tests/scenarios/movement_reliability.json`, not in test code.
- Track held movement and resend the current desired movement at a controlled interval.
- Send movement immediately when desired direction changes.
- Send stop immediately when movement keys are released.
- Reset or allow retry after rejection, disconnect, match restart, and match join.
- Show small player-facing feedback for rejected/blocked input using server reason text.
- Do not spam commands every frame and preserve server-side rate limiting.

## Required quality gates

- Gate A. Every implementation task
- Gate K. Sequential agent pipeline
- Qt quality rules from `docs/agent-rules/quality/QT.md`
- Testing policy from `docs/agent-rules/quality/TESTING.md`

## Required checks

- `cmake -S . -B build -DBATTLE_BUILD_TESTS=ON`
- `cmake --build build --parallel`
- `ctest --test-dir build --output-on-failure`
- `cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_CXX_COMPILER="C:/Program Files/mingw64/bin/c++.exe" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON`
- `cmake --build build-qt-mingw --parallel`
- `ctest --test-dir build-qt-mingw --output-on-failure`
- `python scripts/ci/validate_no_hardcoded_scenarios.py`
- `python scripts/ci/validate_architecture_boundaries.py`
- `python scripts/ci/scan_secrets.py`
- `python scripts/agent/validate_agent_harness.py`
- `python scripts/agent/validate_playable_task_sequence.py`
- `git diff --check`

## Required reviews

- Architecture-Agent: yes
- Qt-Agent: yes
- Security-Agent: no
- Performance-Agent: no
- Frontend-Agent: no
- Verification-Agent: yes

## Acceptance criteria

- Held same-direction movement is resent at a bounded interval.
- Rejected input allows the same held movement to be retried.
- Stop is sent on key release.
- Tests load movement reliability steps from config.
- Qt client compiles with `BATTLE_BUILD_QT_CLIENT=ON`.
- The task is committed on `agent/0047-playable-stabilization` and not merged into `master` or `main`.

## Rollback note

Revert this task commit to restore the previous Qt movement dedupe behavior.
