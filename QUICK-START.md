# Quick Start

## Current Active Test: v16.4 (Task Manager Tail - Pinning Fix)
The current script is the **Final Candidate**. It uses the `ITaskbarList` interface, is optimized for low CPU usage, and includes a fix to correctly handle pinned items.

## How to Run
1. **Open Windhawk**.
2. **Update the mod**: Copy & Paste the contents of the latest version (`v16.4`) into the editor.
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
