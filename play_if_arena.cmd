@echo off
setlocal EnableExtensions

rem IF Arena Qt/TCP local launcher.
rem Put this file in the project root and run it from there.
rem It starts:
rem   1. TCP IF Arena server on 127.0.0.1:5555
rem   2. Two Qt desktop clients

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

set "QT_CLIENT_EXE=%PROJECT_ROOT%\build-qt-mingw\battle_qt_client.exe"

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

if not exist "%QT_CLIENT_EXE%" (
    echo ERROR: battle_qt_client.exe was not found:
    echo   %QT_CLIENT_EXE%
    echo.
    echo Build the Qt client first:
    echo   cmake -S . -B build-qt-mingw -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=ON -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
    echo   cmake --build build-qt-mingw --parallel
    pause
    exit /b 1
)

set "LOCAL_CONFIG=%PROJECT_ROOT%\config\examples\server.local.json"
set "RUNTIME_CONFIG_DIR=%PROJECT_ROOT%\build\run-configs"
set "TCP_CONFIG=%RUNTIME_CONFIG_DIR%\server.tcp.local.json"

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

rem Create/update a TCP-only local runtime config under build/.
rem This avoids writing generated local config files into config/examples/.
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$config = Get-Content '%LOCAL_CONFIG%' -Raw | ConvertFrom-Json; " ^
  "$config.transports.tcp.enabled = $true; " ^
  "$config.transports.tcp.host = '127.0.0.1'; " ^
  "$config.transports.tcp.port = 5555; " ^
  "$config.transports.websocket.enabled = $false; " ^
  "$config | ConvertTo-Json -Depth 20 | Set-Content '%TCP_CONFIG%'"

if errorlevel 1 (
    echo ERROR: failed to create TCP runtime config:
    echo   %TCP_CONFIG%
    pause
    exit /b 1
)

echo Starting IF Arena TCP server...
echo   %SERVER_EXE% --config "%TCP_CONFIG%" --local
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p = Start-Process -FilePath '%SERVER_EXE%' -ArgumentList @('--config','%TCP_CONFIG%','--local') -WorkingDirectory '%PROJECT_ROOT%' -RedirectStandardOutput '%RUN_LOG_DIR%\tcp-server.out.log' -RedirectStandardError '%RUN_LOG_DIR%\tcp-server.err.log' -PassThru; " ^
  "$p.Id | Set-Content '%RUN_STATE_DIR%\tcp-server.pid'; " ^
  "Write-Host ('Started TCP server pid=' + $p.Id)"

echo.
echo Starting two Qt clients...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$p1 = Start-Process -FilePath '%QT_CLIENT_EXE%' -WorkingDirectory '%PROJECT_ROOT%' -PassThru; " ^
  "$p2 = Start-Process -FilePath '%QT_CLIENT_EXE%' -WorkingDirectory '%PROJECT_ROOT%' -PassThru; " ^
  "$p1.Id | Set-Content '%RUN_STATE_DIR%\qt-client-1.pid'; " ^
  "$p2.Id | Set-Content '%RUN_STATE_DIR%\qt-client-2.pid'; " ^
  "Write-Host ('Started Qt clients pid=' + $p1.Id + ', ' + $p2.Id)"

echo.
echo IF Arena Qt/TCP local test is starting.
echo.
echo Client flow:
echo   Client 1: Connect -^> Create
echo   Client 2: Connect -^> enter match code -^> Join
echo.
echo Controls:
echo   WASD / arrows  = move
echo   Mouse          = aim
echo   LMB / Space    = attack
echo   RMB / Shift    = dash
echo.
echo Runtime config generated at:
echo   %TCP_CONFIG%
echo.
echo To stop:
echo   scripts\run\stop_if_arena.cmd
echo Logs:
echo   %RUN_LOG_DIR%
echo.
pause
