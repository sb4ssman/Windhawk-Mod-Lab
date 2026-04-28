# Quick Start

## Current Active Version: v1.0 (Task Manager Tail - Event Driven)
The current script is the **Release Candidate**. It uses `SetWinEventHook` for zero-polling detection and `ITaskbarList` for movement. It is fully configurable via Windhawk settings.

## How to Run
1. **Open Windhawk**.
2. **Update the mod**: Copy & Paste the contents of `task-manager-tail.wh.cpp` into the editor.
3. **Compile & Enable**.

## What to do
1. **Open Task Manager**.
2. **Drag Task Manager** to the middle of the taskbar.
3. **Wait 5 seconds**.
4. Observe if the button disappears and reappears at the end.

## Stress Testing
1. Open PowerShell.
2. Run `.\stress-test.ps1`.
3. Watch the taskbar as 10 Notepads open and close. Task Manager should stay at the end.
