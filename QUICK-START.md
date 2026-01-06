# Quick Start: Running Tests (5 Minutes)

If you just want to get started quickly, here's the fastest path:

## Step 1: Run PowerShell Tests (2 minutes)

1. Open PowerShell (Win+X → Windows PowerShell)

2. Navigate to project:
   ```powershell
   cd "T:\Github\sb4ssman\Windhawk-Taskmanager-Tail"
   ```

3. Allow scripts to run:
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

4. Run the two PowerShell tests:
   ```powershell
   .\test-enumerate-taskbar.ps1
   .\test-find-taskmanager.ps1
   ```
   (Open Task Manager first for the second test)

## Step 2: Test with Windhawk (3 minutes) - **MOST IMPORTANT**

1. **Open Windhawk**

2. **Create new mod** (or open existing one)

3. **Copy entire contents** of `test-hook-explorer.wh.cpp` and paste into Windhawk editor

4. **Compile** (Ctrl+B or click Compile button)

5. **Enable the mod** (toggle it ON)

6. **Open Logs panel** in Windhawk (View → Logs or look for Logs tab)

7. **Open Task Manager** (Ctrl+Shift+Esc)

8. **Watch the logs** - you'll see messages showing what functions are being called!

## What You're Looking For

In the Windhawk logs, look for:
- Messages about `ToolbarWindow32`
- Messages with codes like `0x0417`, `0x0418`, `0x0422`
- Messages that appear when you open Task Manager

These tell us what we need to hook!

## That's It!

The Windhawk test (`test-hook-explorer.wh.cpp`) is the most important one - it will show us exactly what functions to hook.

For more detailed instructions, see `TESTING-GUIDE.md`.
