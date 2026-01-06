# Windhawk-Taskmanager-Tail

Windhawk mod to automatically keep Task Manager as the last item on the taskbar.

## Research Summary

### Existing Windhawk Modules Analysis

After researching available Windhawk modules, **none provide the functionality needed**:

1. **Windows 11 Taskbar Styler** - Only handles visual customization (themes, colors, styles). Does NOT affect taskbar button ordering.
2. **Taskbar Thumbnail Reorder** - Only reorders thumbnails within grouped taskbar items. Does NOT affect the main taskbar button order.

**Conclusion**: A custom Windhawk mod is required.

### Event-Based Approach (Preferred)

Instead of polling, use **Windhawk's event-driven hooks**:

1. **Hook into taskbar functions** in `explorer.exe` that handle button ordering
2. **Intercept button creation/reordering** events
3. **When Task Manager button is detected**, automatically move it to the end
4. **No polling needed** - responds immediately to events

This approach:
- ✅ Event-driven, no polling overhead
- ✅ Immediate response to taskbar changes
- ✅ Uses Windhawk's built-in hooking capabilities
- ✅ More efficient and responsive

## Technical Implementation Plan

### Architecture

1. **Target Process**: Hook into `explorer.exe` (manages the taskbar)
2. **Event-Based Hooking**: Use `Wh_SetFunctionHook` to intercept taskbar functions
3. **Taskbar Manipulation**: Modify button order when Task Manager is detected

### Key Technical Details

#### Finding the Taskbar
- Taskbar window: `FindWindow("Shell_TrayWnd", NULL)`
- Toolbar control: Child window with class `ToolbarWindow32`

#### Identifying Task Manager
- Process name: `Taskmgr.exe`
- Window class: `TaskManagerWindow` (Windows 11)
- Can enumerate taskbar buttons and match by process/window

#### Reordering Buttons
- Use `TB_MOVEBUTTON` message to move toolbar buttons
- Find Task Manager button index
- Move it to the last position (button count - 1)

### Windhawk API Functions

Windhawk provides these key APIs for mods:

- **`Wh_SetFunctionHook`** - Hook into functions within the target process
- **`Wh_ModInit`** - Called when mod is loaded (setup hooks here)
- **`Wh_ModUninit`** - Called when mod is unloaded (cleanup)
- **`Wh_Log`** - Logging for debugging
- **`Wh_GetStringSetting`** / **`Wh_FreeStringSetting`** - Settings management

### Windhawk Mod Structure

```cpp
// ==WindhawkMod==
// @id           taskmanager-tail
// @name         Task Manager Tail
// @description  Keeps Task Manager as the last item on the taskbar
// @version      1.0
// @author       Your Name
// @include      explorer.exe
// ==/WindhawkMod==

#include <windhawk.h>

// Hook function type for taskbar button ordering
typedef void (WINAPI *TaskbarButtonOrderFunc)(...);

// Our hook implementation
void WINAPI TaskbarButtonOrderHook(...) {
    // Call original function first
    TaskbarButtonOrderFunc originalFunc = (TaskbarButtonOrderFunc)Wh_GetOriginalFunction();
    originalFunc(...);
    
    // Then move Task Manager to end if it exists
    MoveTaskManagerToEnd();
}

BOOL Wh_ModInit() {
    // Hook into the function that handles taskbar button ordering
    // Need to identify the actual function name in explorer.exe
    Wh_SetFunctionHook(
        (void*)GetProcAddress(GetModuleHandle(L"explorer.exe"), "FunctionName"),
        (void*)TaskbarButtonOrderHook,
        nullptr
    );
    return TRUE;
}

void Wh_ModUninit() {
    // Cleanup hooks
}
```

### Event-Based Approaches

**Option 1: Hook Taskbar Button Creation Function**
- Hook the internal `explorer.exe` function that adds buttons to the taskbar
- When a button is added, check if it's Task Manager and move it to end

**Option 2: Subclass Taskbar Window Procedure**
- Use `SetWindowLongPtr` to subclass the taskbar window (`Shell_TrayWnd`)
- Intercept window messages related to button creation/reordering
- Respond to `WM_COMMAND` or custom taskbar messages

**Option 3: Hook Window Creation**
- Hook `CreateWindowEx` or similar in `explorer.exe`
- Detect when Task Manager window is created
- Trigger taskbar reordering

**Option 4: Hook Taskbar Update Functions**
- Identify internal taskbar update/refresh functions
- Hook them to intercept when taskbar buttons are reordered
- Force Task Manager to end position after updates

### Implementation Steps

1. **Research Internal Functions**: Use tools like IDA Pro or x64dbg to identify the actual function names in `explorer.exe` that handle:
   - Taskbar button creation
   - Taskbar button ordering
   - Taskbar updates/refreshes

2. **Create mod structure** with metadata targeting `explorer.exe`

3. **Hook the appropriate function** using `Wh_SetFunctionHook`:
   - Hook function that adds buttons to taskbar, OR
   - Hook function that reorders taskbar buttons, OR
   - Hook taskbar refresh/update function

4. **Implement detection logic** in hook:
   - Identify Task Manager button (by process name or window handle)
   - Move it to end using `TB_MOVEBUTTON` or internal API

5. **Alternative: Subclass taskbar window**:
   - Find `Shell_TrayWnd` window
   - Subclass its window procedure
   - Intercept relevant messages and reorder when needed

### Resources

- [Windhawk Mod Development Guide](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windhawk Development Tips](https://github.com/ramensoftware/windhawk/wiki/Development-tips)
- [Windhawk Mods Repository](https://github.com/ramensoftware/windhawk-mods) - for code examples
- Windows API: `TB_MOVEBUTTON`, `FindWindow`, `EnumChildWindows`, `SetWindowLongPtr`

### Next Steps

1. **Reverse Engineer Explorer.exe**: Identify the actual function names that handle taskbar button ordering
2. **Study Existing Mods**: Review taskbar-related mods in the repository for patterns
3. **Implement Hook**: Use `Wh_SetFunctionHook` to intercept the identified function
4. **Test**: Ensure it works with two-line taskbar and doesn't cause instability

## Files Created

### Main Mod
- **`taskmanager-tail.wh.cpp`** - Main Windhawk mod skeleton with:
  - Proper metadata structure
  - Task Manager window detection functions
  - Taskbar toolbar finding functions
  - Placeholder hook functions (to be filled in after research)
  - Basic button reordering logic

### Test Scripts
- **`test-enumerate-taskbar.ps1`** - PowerShell script to enumerate taskbar toolbars and button counts
- **`test-find-taskmanager.ps1`** - PowerShell script to find Task Manager windows by class name and process
- **`test-taskbar-buttons.cpp`** - C++ test program to enumerate taskbar buttons (compile and run standalone)
- **`test-hook-explorer.wh.cpp`** - Windhawk test mod to hook and log explorer.exe functions:
  - Logs window creation (especially taskbar-related)
  - Logs toolbar messages (TB_* messages)
  - Use this to identify what functions/messages are involved

### Example
- **`EXAMPLE-windhawk-modcpp.cpp`** - Reference example mod structure

## Next Steps

1. **Run test scripts** - See `TESTING-GUIDE.md` for detailed instructions:
   - **Easy**: Run `test-enumerate-taskbar.ps1` (PowerShell) - see toolbar structure
   - **Easy**: Run `test-find-taskmanager.ps1` (PowerShell) - verify Task Manager detection
   - **Easy**: Load `test-hook-explorer.wh.cpp` in Windhawk - **MOST IMPORTANT** - shows what functions are called
   - **Optional**: Compile and run `test-taskbar-buttons.cpp` (C++) - enumerate buttons

2. **Review the logs** from `test-hook-explorer.wh.cpp`:
   - Open Task Manager and watch Windhawk logs
   - Identify what messages/functions are called when Task Manager opens
   - This will tell us exactly what to hook

3. **Update taskmanager-tail.wh.cpp**:
   - Replace placeholder hooks with real function hooks based on test results
   - Implement the actual reordering logic

4. **Test the mod**:
   - Enable the mod in Windhawk
   - Open Task Manager
   - Verify it moves to the end of the taskbar
   - Test with two-line taskbar

## Status

**Development Phase** - Mod skeleton and test scripts created. Ready to:
1. Run test scripts to probe taskbar behavior
2. Use test-hook-explorer mod to identify functions to hook
3. Research internal explorer.exe functions
4. Complete the implementation
