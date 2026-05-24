# Release Notes: v0.2.1-playable-alpha

Status: release candidate notes. Do not create the git tag until explicitly authorized.

## Highlights

- Local playable Objective Run alpha with authoritative C++20 backend and deterministic `battle_core`.
- Raw TCP path for CLI, Qt, and load clients remains local-demo focused.
- WebSocket path for browser/Telegram Mini App local development remains strict and versioned.
- Qt and Web clients render the production SVG player asset with team/carrier overlays.
- Architecture-boundary documentation and validator are active.
- PR and main CI validate C++, TypeScript, structure, secret scanning, and architecture boundaries.

## Verified Commands

Default C++:

```powershell
cmake -S . -B build -DBATTLE_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Frontend:

```powershell
cd frontend\telegram_mini_app
npm.cmd run typecheck
npm.cmd run lint
npm.cmd run build
```

Repository validation:

```powershell
python scripts\ci\validate_architecture_boundaries.py
python scripts\ci\scan_secrets.py
python scripts\agent\validate_agent_harness.py
python scripts\agent\validate_playable_task_sequence.py
```

Qt MinGW, when Qt is installed:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"
cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
cmake --build build-qt-mingw --parallel
ctest --test-dir build-qt-mingw --output-on-failure
```

## Known Limitations

- Local playable alpha only; not a production deployment.
- Public Telegram usage still requires HTTPS/WSS termination and deployment review.
- Replay protection and production session-token issuance remain follow-up work.
- Raw TCP should stay local unless a release explicitly adds firewalling and operational controls.
- Large slow-reader soaks, mixed TCP/WebSocket load, snapshot coalescing, and production metrics export remain future hardening work.
- Browser E2E automation is planned for task 0038.

## Tagging

Recommended tag after explicit user approval: `v0.2.1-playable-alpha`.

No tag or push is created by task 0037.
