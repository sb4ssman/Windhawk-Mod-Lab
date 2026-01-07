@echo off
echo ================================================
echo Windhawk Task Manager Tail - All Tests Runner
echo ================================================
echo.

echo Setting PowerShell execution policy...
powershell Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
echo.

echo Running Taskbar Enumeration Test...
echo ================================================
powershell -ExecutionPolicy Bypass -File .\test-enumerate-taskbar.ps1 > test-enumerate-taskbar.log 2>&1
echo.
echo Output saved to test-enumerate-taskbar.log
echo.

echo Running Task Manager Detection Test...
echo ============================================
powershell -ExecutionPolicy Bypass -File .\test-find-taskmanager.ps1 > test-find-taskmanager.log 2>&1
echo.
echo Output saved to test-find-taskmanager.log
echo.

echo ================================================
echo Next Steps:
echo 1. Open Windhawk
echo 2. Create a new mod or open the existing test-hook-explorer.wh.cpp
echo 3. Copy the contents of test-hook-explorer.wh.cpp into Windhawk
echo 4. Compile the mod (Ctrl+B)
echo 5. Enable the mod
echo 6. Open Task Manager (Ctrl+Shift+Esc)
echo 7. Watch the Windhawk logs for taskbar activity
echo 8. Note the function names and messages for the main mod implementation
echo.

echo For C++ Test (optional):
echo 1. Compile: cl test-taskbar-buttons.cpp /link user32.lib comctl32.lib
echo 2. Run: test-taskbar-buttons.exe
echo.

echo All tests completed. Check the log files for results.
echo ============================================
