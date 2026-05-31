@echo off
setlocal EnableExtensions

set "KEEP_VITE=0"
if /I "%~1"=="--keep-vite" set "KEEP_VITE=1"

cd /d "%~dp0\..\.."
set "PROJECT_ROOT=%CD%"
set "RUN_STATE_DIR=%PROJECT_ROOT%\build\run-state"

echo [IF Arena] Stopping old IF Arena processes...

if exist "%RUN_STATE_DIR%" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
      "$stateDir = '%RUN_STATE_DIR%'; " ^
      "$keepVite = '%KEEP_VITE%' -eq '1'; " ^
      "Get-ChildItem -LiteralPath $stateDir -Filter '*.pid' -ErrorAction SilentlyContinue | ForEach-Object { " ^
      "  $pidText = (Get-Content -LiteralPath $_.FullName -ErrorAction SilentlyContinue | Select-Object -First 1); " ^
      "  $pidValue = 0; " ^
      "  if (-not [int]::TryParse([string]$pidText, [ref]$pidValue)) { return; } " ^
      "  try { " ^
      "    $p = Get-Process -Id $pidValue -ErrorAction Stop; " ^
      "    if ($keepVite -and $_.Name -like '*vite*') { return; } " ^
      "    if ($p.ProcessName -in @('battle_server_app', 'battle_qt_client', 'node', 'npm')) { " ^
      "      Write-Host ('Stopping pid-file process ' + $p.ProcessName + ' pid=' + $pidValue); " ^
      "      Stop-Process -Id $pidValue -Force -ErrorAction SilentlyContinue; " ^
      "    } " ^
      "  } catch {} " ^
      "  Remove-Item -LiteralPath $_.FullName -Force -ErrorAction SilentlyContinue; " ^
      "}"
)

taskkill /F /T /IM battle_server_app.exe >nul 2>nul
if not errorlevel 1 echo [IF Arena] Stopped battle_server_app.exe

taskkill /F /T /IM battle_qt_client.exe >nul 2>nul
if not errorlevel 1 echo [IF Arena] Stopped battle_qt_client.exe

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$keepVite = '%KEEP_VITE%' -eq '1'; " ^
  "$ports = if ($keepVite) { @(5555, 8081) } else { @(5555, 8081, 5173) }; " ^
  "$owners = Get-NetTCPConnection -LocalPort $ports -ErrorAction SilentlyContinue | Select-Object -ExpandProperty OwningProcess -Unique; " ^
  "foreach ($pidValue in $owners) { " ^
  "  try { " ^
  "    $p = Get-Process -Id $pidValue -ErrorAction Stop; " ^
  "    if ($p.ProcessName -in @('battle_server_app', 'battle_qt_client', 'node')) { " ^
  "      Write-Host ('Stopping port owner ' + $p.ProcessName + ' pid=' + $pidValue); " ^
  "      Stop-Process -Id $pidValue -Force -ErrorAction SilentlyContinue; " ^
  "    } else { " ^
  "      Write-Host ('Leaving unrelated port owner ' + $p.ProcessName + ' pid=' + $pidValue); " ^
  "    } " ^
  "  } catch {} " ^
  "}"

if "%KEEP_VITE%"=="1" (
    echo [IF Arena] Old server/client processes stopped. Vite on 5173 was kept.
) else (
    echo [IF Arena] Old IF Arena server/client/Vite processes stopped.
)

endlocal
