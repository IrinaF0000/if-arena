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

rem Make Qt/MinGW DLLs visible for MinGW-built executables.
set "PATH=%QT_DIR%\bin;%MINGW_BIN%;%NINJA_DIR%;%PATH%"

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
start "IF Arena TCP Server" cmd /k ""%SERVER_EXE%" --config "%TCP_CONFIG%" --local"

echo.
echo Starting two Qt clients...
start "IF Arena Qt Client 1" cmd /k "cd /d "%PROJECT_ROOT%" && "%QT_CLIENT_EXE%""
start "IF Arena Qt Client 2" cmd /k "cd /d "%PROJECT_ROOT%" && "%QT_CLIENT_EXE%""

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
echo   close Qt clients, then press Ctrl+C in the server window.
echo.
pause
