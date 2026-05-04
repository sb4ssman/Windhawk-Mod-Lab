@echo off
setlocal

set "ROOT=%~dp0"
set "ROOT=%ROOT:~0,-1%"
set "PORT=8765"
set "URL=http://127.0.0.1:%PORT%/privacy-trigger-test.html"

powershell -NoProfile -ExecutionPolicy Bypass -Command "try { $client = [Net.Sockets.TcpClient]::new('127.0.0.1', %PORT%); $client.Close(); exit 0 } catch { exit 1 }" >nul 2>&1
if errorlevel 1 (
    start "Privacy Indicator Trigger Server" powershell -NoExit -NoProfile -ExecutionPolicy Bypass -File "%ROOT%\privacy-trigger-server.ps1" -Root "%ROOT%" -Port %PORT%
)

for /l %%i in (1,1,20) do (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "try { $client = [Net.Sockets.TcpClient]::new('127.0.0.1', %PORT%); $client.Close(); exit 0 } catch { exit 1 }" >nul 2>&1
    if not errorlevel 1 goto open_page
    timeout /t 1 /nobreak >nul
)

:open_page
start "" "%URL%"
