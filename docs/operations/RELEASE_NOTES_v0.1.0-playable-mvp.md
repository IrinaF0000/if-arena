# Release Notes: v0.1.0-playable-mvp

Status: release candidate notes. Do not create the git tag until explicitly authorized.

## Highlights

- Authoritative C++20 Objective Run backend with deterministic core simulation.
- Local raw TCP vertical slice for CLI, Qt, and load clients.
- Local WebSocket slice for Telegram Mini App development.
- Qt Widgets desktop client over raw TCP, verified through the Qt MinGW build path.
- Telegram Mini App frontend with strict TypeScript checks and runtime message validation.
- Reproducible local load/security smoke scripts and an honest local load report.

## Verified Commands

Default C++:

```powershell
cmake -S . -B build -DBATTLE_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Qt MinGW:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;$env:Path"
cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
cmake --build build-qt-mingw --parallel
ctest --test-dir build-qt-mingw --output-on-failure
```

Frontend:

```powershell
cd frontend\telegram_mini_app
npm run typecheck
npm run lint
npm run build
```

Load and security:

```powershell
python tests\load\load_client_dry_run.py
python tests\load\local_tcp_load_scenarios.py --report build\local-tcp-smoke.md
python tests\security\tcp_protocol_negative.py
python tests\frontend\telegram_protocol_validation.mjs
python tests\frontend\websocket_local_smoke.py
python scripts\ci\scan_secrets.py
```

## Known Limitations

- Local playable MVP only; not a production deployment.
- Public Telegram usage still requires HTTPS/WSS termination and deployment review.
- Telegram auth freshness and signature checks exist, but replay protection and production session-token issuance remain follow-up work.
- Raw TCP should stay local unless a release explicitly adds firewalling and operational controls.
- Large slow-reader soak, mixed TCP/WebSocket load, snapshot coalescing, and production metrics export remain future hardening work.

## Tagging

Suggested tag name: `v0.1.0-playable-mvp`.

Tagging must be a separate explicit action after reviewing these notes and final checks.
