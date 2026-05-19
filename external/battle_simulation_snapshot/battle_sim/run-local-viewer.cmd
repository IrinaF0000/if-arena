@echo off
setlocal

rem Starts the local replay viewer service and opens the browser UI.
rem Place this file in the repository root and double-click it, or run it from cmd.
rem Optional usage:
rem   run-local-viewer.cmd path\to\battle_sim.exe

cd /d "%~dp0"

set "SERVER=tools\local-runner\server.py"
set "URL=http://127.0.0.1:8765"

if not exist "%SERVER%" (
    echo ERROR: Cannot find %SERVER%
    echo Make sure this script is placed in the repository root.
    pause
    exit /b 1
)

set "EXE="

if not "%~1"=="" (
    set "EXE=%~1"
) else (
    for %%F in (
        "build\Release\battle_sim.exe"
        "build\RelWithDebInfo\battle_sim.exe"
        "build\Debug\battle_sim.exe"
        "build\battle_sim.exe"
    ) do (
        if not defined EXE if exist %%~F set "EXE=%%~F"
    )
)

if not defined EXE (
    echo ERROR: Cannot find battle_sim.exe.
    echo.
    echo Build the project first, for example:
    echo   cmake -S . -B build
    echo   cmake --build build --config Release
    echo.
    echo Or pass the executable path explicitly:
    echo   run-local-viewer.cmd build\Release\battle_sim.exe
    pause
    exit /b 1
)

where py >nul 2>nul
if %ERRORLEVEL%==0 (
    set "PY=py -3"
) else (
    where python >nul 2>nul
    if %ERRORLEVEL%==0 (
        set "PY=python"
    ) else (
        echo ERROR: Python was not found.
        echo Install Python 3 or add it to PATH.
        pause
        exit /b 1
    )
)

echo Starting local replay viewer...
echo Executable: %EXE%
echo URL:        %URL%
echo.
echo Keep this window open while using the viewer.
echo Press Ctrl+C in this window to stop the service.
echo.

rem Open the browser shortly after the service starts.
start "" /min cmd /c "timeout /t 2 /nobreak >nul & start "" %URL%"

%PY% "%SERVER%" --exe "%EXE%"

echo.
echo Local replay viewer stopped.
pause
