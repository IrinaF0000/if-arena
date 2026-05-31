@echo off
setlocal EnableExtensions

rem IF Arena WebSocket / Telegram Mini App local launcher.
rem Put this file in the project root and run it from there.
rem It starts:
rem   1. WebSocket IF Arena server on ws://127.0.0.1:8081/ws
rem   2. Vite frontend on http://127.0.0.1:5173
rem   3. Two browser tabs for local two-client testing

cd /d "%~dp0"

set "PROJECT_ROOT=%CD%"
set "QT_DIR=C:\Qt\6.11.1\mingw_64"
set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"
set "NINJA_DIR=C:\Qt\Tools\Ninja"
set "STOP_SCRIPT=%PROJECT_ROOT%\scripts\run\stop_if_arena.cmd"
set "RUN_LOG_DIR=%PROJECT_ROOT%\build\run-logs"
set "RUN_STATE_DIR=%PROJECT_ROOT%\build\run-state"

rem Make Qt/MinGW DLLs visible for MinGW-built executables.
set "PATH=%QT_DIR%\bin;%MINGW_BIN%;%NINJA_DIR%;%PATH%"

if exist "%STOP_SCRIPT%" (
    call "%STOP_SCRIPT%"
)

set "SERVER_EXE=%PROJECT_ROOT%\build-qt-mingw\battle_server_app.exe"
if not exist "%SERVER_EXE%" (
    set "SERVER_EXE=%PROJECT_ROOT%\build\battle_server_app.exe"
)

if not exist "%SERVER_EXE%" (
    echo ERROR: battle_server_app.exe was not found.
    echo Expected one of:
    echo   %PROJECT_ROOT%\build-qt-mingw\battle_server_app.exe
    echo   %PROJECT_ROOT%\build\battle_server_app.exe
    echo.
    echo Build the project first, then run this script again.
    pause
    exit /b 1
)

set "LOCAL_CONFIG=%PROJECT_ROOT%\config\examples\server.local.json"
set "RUNTIME_CONFIG_DIR=%PROJECT_ROOT%\build\run-configs"
set "WS_CONFIG=%RUNTIME_CONFIG_DIR%\server.ws.local.json"

if not exist "%LOCAL_CONFIG%" (
    echo ERROR: %LOCAL_CONFIG% was not found.
    pause
    exit /b 1
)

if not exist "%RUNTIME_CONFIG_DIR%" (
    mkdir "%RUNTIME_CONFIG_DIR%"
)
if not exist "%RUN_LOG_DIR%" (
    mkdir "%RUN_LOG_DIR%"
)
if not exist "%RUN_STATE_DIR%" (
    mkdir "%RUN_STATE_DIR%"
)

rem Create/update a WebSocket-only local runtime config under build/.
rem TCP must be disabled, otherwise battle_server_app starts TCP instead of WebSocket.
rem This avoids writing generated local config files into config/examples/.
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$config = Get-Content '%LOCAL_CONFIG%' -Raw | ConvertFrom-Json; " ^
  "$config.transports.tcp.enabled = $false; " ^
  "$config.transports.websocket.enabled = $true; " ^
  "$config.transports.websocket.host = '127.0.0.1'; " ^
  "$config.transports.websocket.port = 8081; " ^
  "$config.transports.websocket.path = '/ws'; " ^
  "$config | ConvertTo-Json -Depth 20 | Set-Content '%WS_CONFIG%'"

if errorlevel 1 (
    echo ERROR: failed to create WebSocket runtime config:
    echo   %WS_CONFIG%
    pause
    exit /b 1
)

set "FRONTEND_DIR=%PROJECT_ROOT%\frontend\telegram_mini_app"
if not exist "%FRONTEND_DIR%\package.json" (
    echo ERROR: frontend package.json was not found:
    echo   %FRONTEND_DIR%\package.json
    pause
    exit /b 1
)

where npm.cmd >nul 2>nul
if errorlevel 1 (
    echo ERROR: npm.cmd was not found in PATH.
    echo Install Node.js LTS or open a terminal where npm.cmd is available.
    pause
    exit /b 1
)

echo Starting IF Arena WebSocket server...
echo   %SERVER_EXE% --config "%WS_CONFIG%" --local
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p = Start-Process -FilePath '%SERVER_EXE%' -ArgumentList @('--config','%WS_CONFIG%','--local') -WorkingDirectory '%PROJECT_ROOT%' -RedirectStandardOutput '%RUN_LOG_DIR%\ws-server.out.log' -RedirectStandardError '%RUN_LOG_DIR%\ws-server.err.log' -PassThru; " ^
  "$p.Id | Set-Content '%RUN_STATE_DIR%\ws-server.pid'; " ^
  "Write-Host ('Started WebSocket server pid=' + $p.Id)"

echo.
echo Starting Telegram Mini App frontend...
echo   VITE_WS_URL=ws://127.0.0.1:8081/ws
echo   URL=http://127.0.0.1:5173/
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$env:VITE_WS_URL = 'ws://127.0.0.1:8081/ws'; " ^
  "$p = Start-Process -FilePath 'npm.cmd' -ArgumentList @('run','dev','--','--host','127.0.0.1','--port','5173') -WorkingDirectory '%FRONTEND_DIR%' -RedirectStandardOutput '%RUN_LOG_DIR%\vite.out.log' -RedirectStandardError '%RUN_LOG_DIR%\vite.err.log' -PassThru; " ^
  "$p.Id | Set-Content '%RUN_STATE_DIR%\vite.pid'; " ^
  "Write-Host ('Started Vite pid=' + $p.Id)"

echo.
echo Waiting a few seconds before opening browser tabs...
timeout /t 5 /nobreak >nul

start "" "http://127.0.0.1:5173/"
start "" "http://127.0.0.1:5173/"

echo.
echo IF Arena WebSocket local test is starting.
echo.
echo Browser flow:
echo   Tab 1: Connect -^> Create
echo   Tab 2: Connect -^> enter match code -^> Join
echo.
echo Expected backend:
echo   ws://127.0.0.1:8081/ws
echo.
echo Runtime config generated at:
echo   %WS_CONFIG%
echo.
echo To stop:
echo   close browser tabs, then run scripts\run\stop_if_arena.cmd
echo   use scripts\run\stop_if_arena.cmd --keep-vite to keep Vite on 5173 running.
echo Logs:
echo   %RUN_LOG_DIR%
echo.
pause
