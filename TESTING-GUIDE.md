# Testing Guide: How to Run Test Scripts

This guide will walk you through running each test script to probe the taskbar and understand how it works.

## Prerequisites

- Windows 11
- PowerShell (comes with Windows)
- Windhawk installed (for the Windhawk mod test)
- Optional: C++ compiler (for the C++ test - we'll provide alternatives)

---

## Test 1: Enumerate Taskbar Toolbars (PowerShell)

**File**: `test-enumerate-taskbar.ps1`

### What This Does
This script finds the taskbar window and lists all toolbar controls within it, showing how many buttons each toolbar has.

### How to Run

1. **Open PowerShell**:
   - Press `Win + X` and select "Windows PowerShell" or "Terminal"
   - Or press `Win + R`, type `powershell`, and press Enter

2. **Navigate to the project folder**:
   ```powershell
   cd "T:\Github\sb4ssman\Windhawk-Taskmanager-Tail"
   ```

3. **Run the script**:
   ```powershell
   .\test-enumerate-taskbar.ps1
   ```

   If you get an execution policy error, run this first:
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```

### What to Expect

You should see output like:
```
=== Taskbar Enumeration Test ===
Taskbar window handle: 0x00012345

Toolbar #0
  Handle: 0x00012346
  Class: ToolbarWindow32
  Button count: 8
  This appears to be the main taskbar toolbar

Toolbar #1
  Handle: 0x00012347
  Class: ToolbarWindow32
  Button count: 0

=== Test Complete ===
```

**What This Tells Us**: 
- The taskbar structure (how many toolbars exist)
- Which toolbar has the actual buttons (button count > 0)
- The handle we'll need to manipulate buttons

---

## Test 2: Find Task Manager Windows (PowerShell)

**File**: `test-find-taskmanager.ps1`

### What This Does
This script scans all open windows to find Task Manager windows by checking:
- Window class name (`TaskManagerWindow`)
- Process name (`Taskmgr.exe`)

### How to Run

1. **Open PowerShell** (same as above)

2. **Navigate to the project folder**:
   ```powershell
   cd "T:\Github\sb4ssman\Windhawk-Taskmanager-Tail"
   ```

3. **IMPORTANT**: First, open Task Manager:
   - Press `Ctrl + Shift + Esc` to open Task Manager
   - Or right-click the taskbar and select "Task Manager"
   - Keep it open while running the test

4. **Run the script**:
   ```powershell
   .\test-find-taskmanager.ps1
   ```

### What to Expect

If Task Manager is running, you'll see:
```
=== Task Manager Window Detection Test ===

Searching for Task Manager windows...

=== Task Manager Window Found ===
Handle: 0x00012348
Class: TaskManagerWindow
Title: Task Manager
Process: Taskmgr.exe (PID: 12345)

=== Test Complete ===
```

If Task Manager is NOT running:
```
Task Manager is not currently running.
Please open Task Manager and run this script again.
```

**What This Tells Us**:
- That we can correctly identify Task Manager windows
- The window class name to look for
- The process name to verify

---

## Test 3: Enumerate Taskbar Buttons (C++)

**File**: `test-taskbar-buttons.cpp`

### What This Does
This C++ program finds the taskbar toolbar and lists all buttons, showing their text labels. This helps us see what buttons exist and find Task Manager's button.

### Option A: Compile with MSVC (Visual Studio)

1. **Install Visual Studio Build Tools** (if you don't have Visual Studio):
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Select "Build Tools for Visual Studio"
   - Install "Desktop development with C++" workload

2. **Open "Developer Command Prompt for VS"**:
   - Search for "Developer Command Prompt" in Start menu
   - Or open regular PowerShell and run:
     ```powershell
     & "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
     ```
     (Adjust path for your VS version)

3. **Navigate to project folder**:
   ```cmd
   cd "T:\Github\sb4ssman\Windhawk-Taskmanager-Tail"
   ```

4. **Compile**:
   ```cmd
   cl test-taskbar-buttons.cpp /link user32.lib comctl32.lib
   ```

5. **Run**:
   ```cmd
   test-taskbar-buttons.exe
   ```

### Option B: Compile with MinGW (Easier Alternative)

1. **Install MinGW-w64**:
   - Download from: https://www.mingw-w64.org/downloads/
   - Or use MSYS2: https://www.msys2.org/
   - Or use Chocolatey: `choco install mingw`

2. **Open PowerShell or Command Prompt**

3. **Navigate to project folder**:
   ```powershell
   cd "T:\Github\sb4ssman\Windhawk-Taskmanager-Tail"
   ```

4. **Compile**:
   ```powershell
   g++ -o test-taskbar-buttons.exe test-taskbar-buttons.cpp -lgdi32 -luser32 -lcomctl32
   ```

5. **Run**:
   ```powershell
   .\test-taskbar-buttons.exe
   ```

### Option C: Use Online Compiler (No Installation)

If you don't want to install a compiler, you can use an online C++ compiler:

1. Go to: https://www.onlinegdb.com/online_c++_compiler
2. Copy the contents of `test-taskbar-buttons.cpp`
3. Paste into the editor
4. Click "Run"
5. **Note**: This won't work perfectly because it can't access Windows APIs, but you can see the code structure

### What to Expect

You should see output like:
```
=== Taskbar Button Enumeration Test ===

Found taskbar window: 0x00012345

Toolbar #0: handle=0x00012346, buttons=8
  -> This appears to be the main taskbar toolbar

Enumerating 8 buttons...
  Button #0: "Microsoft Edge"
  Button #1: "File Explorer"
  Button #2: "Visual Studio Code"
  Button #3: "Task Manager"
  Button #4: "Settings"
  ...

Found 8 buttons total
Task Manager found at index 3
```

**What This Tells Us**:
- How many buttons are on the taskbar
- What text labels the buttons have
- Where Task Manager's button is located (index)
- That we can enumerate buttons programmatically

---

## Test 4: Hook Explorer Functions (Windhawk Mod)

**File**: `test-hook-explorer.wh.cpp`

### What This Does
This Windhawk mod hooks into `explorer.exe` and logs:
- When windows are created (especially taskbar-related windows)
- When messages are sent to toolbar controls (like `TB_MOVEBUTTON`, `TB_GETBUTTON`, etc.)

This is the **most important test** because it will show us what functions are actually being called when Task Manager opens.

### How to Run

1. **Open Windhawk**

2. **Create a new mod**:
   - Click the "+" button or "New Mod"
   - Or go to File → New Mod

3. **Copy the test mod code**:
   - Open `test-hook-explorer.wh.cpp` in a text editor
   - Select all (Ctrl+A) and copy (Ctrl+C)

4. **Paste into Windhawk editor**:
   - In Windhawk's editor, select all existing code
   - Paste (Ctrl+V) to replace it

5. **Compile the mod**:
   - Click the "Compile" button (usually on the left side)
   - Or press `Ctrl+B`
   - Wait for "Compilation successful" message

6. **Enable the mod**:
   - The mod should appear in your mod list
   - Toggle it ON (enable it)

7. **Open the log viewer**:
   - In Windhawk, look for a "Logs" or "Output" tab/panel
   - Or go to View → Logs
   - This will show `Wh_Log` output

8. **Test the mod**:
   - **Before opening Task Manager**, check the logs - you might see some initial messages
   - **Open Task Manager** (Ctrl+Shift+Esc)
   - **Watch the logs** - you should see messages like:
     ```
     CreateWindowExW: Class=TaskManagerWindow, Title=Task Manager, Handle=0x00012348
     SendMessageW to ToolbarWindow32: Msg=0x0417, wParam=0x00000000, lParam=0x00000000
     SendMessageW to ToolbarWindow32: Msg=0x0418, wParam=0x00000000, lParam=0x00000000
     ```

9. **Open and close some apps**:
   - Open a few applications (Notepad, Calculator, etc.)
   - Watch the logs to see what messages are sent to the toolbar
   - Close some apps and see what happens

### What to Expect

The logs will show:
- **Window creation**: When Task Manager or other windows are created
- **Toolbar messages**: What messages are sent to the taskbar toolbar
  - `0x0417` = `TB_GETBUTTON` (get button info)
  - `0x0418` = `TB_BUTTONCOUNT` (get button count)
  - `0x0422` = `TB_MOVEBUTTON` (move button - if we see this, we know reordering happens!)
  - `0x0413` = `TB_INSERTBUTTON` (add button)
  - `0x0416` = `TB_DELETEBUTTON` (remove button)

**What This Tells Us**:
- What functions/messages are involved in taskbar operations
- When Task Manager gets added to the taskbar
- Whether Windows uses `TB_MOVEBUTTON` or something else
- What we need to hook to intercept button ordering

### Interpreting the Logs

Look for patterns like:
- When you open Task Manager, what messages appear?
- When you open another app, what messages appear?
- Is there a specific sequence of messages that happens?

This will help us identify:
1. **What function to hook** - If we see `TB_MOVEBUTTON`, we know Windows uses that
2. **When to trigger our code** - We can hook the function that sends these messages
3. **What parameters are used** - We can see the button indices

---

## Troubleshooting

### PowerShell Scripts Won't Run

**Error**: "execution of scripts is disabled"

**Solution**:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### C++ Compilation Errors

**Error**: "cl is not recognized"

**Solution**: You need to open "Developer Command Prompt for VS" or run `vcvars64.bat` first.

**Error**: "g++ is not recognized"

**Solution**: Install MinGW-w64 and add it to your PATH, or use the full path to g++.

### Windhawk Mod Won't Compile

**Error**: Compilation fails

**Solution**:
- Make sure you copied the entire file, including the metadata section
- Check that Windhawk is up to date
- Look at the error message - it will tell you what's wrong

### No Logs Appear in Windhawk

**Solution**:
- Make sure the mod is enabled (toggle is ON)
- Restart explorer.exe or reboot
- Check that you're looking at the right log panel
- Try adding a log message in `Wh_ModInit` to verify logging works

### Task Manager Not Found

**Solution**:
- Make sure Task Manager is actually running
- Try opening it fresh (close and reopen)
- Check that you're looking at the right window (there might be multiple Task Manager windows)

---

## What to Do With the Results

After running all tests:

1. **Document what you found**:
   - How many toolbars exist
   - How many buttons are on the taskbar
   - What messages are sent when Task Manager opens
   - What the button index of Task Manager is

2. **Share the findings**:
   - The logs from `test-hook-explorer.wh.cpp` are especially valuable
   - They'll help identify what functions we need to hook

3. **Next steps**:
   - Once we know what functions/messages are involved, we can update `taskmanager-tail.wh.cpp`
   - We'll replace the placeholder hooks with real function hooks
   - We'll implement the actual reordering logic

---

## Quick Reference

| Test | File | Purpose | Difficulty |
|------|------|---------|------------|
| 1 | `test-enumerate-taskbar.ps1` | Find taskbar structure | Easy |
| 2 | `test-find-taskmanager.ps1` | Find Task Manager windows | Easy |
| 3 | `test-taskbar-buttons.cpp` | List all taskbar buttons | Medium (needs compiler) |
| 4 | `test-hook-explorer.wh.cpp` | Log explorer.exe activity | Easy (in Windhawk) |

**Start with tests 1, 2, and 4** - they're the easiest and most informative. Test 3 is optional but helpful.
