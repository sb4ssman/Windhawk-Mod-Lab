# Research Summary: Task Manager Tail Mod

## Research Findings

### Existing Windhawk Modules
- **Windows 11 Taskbar Styler**: Visual customization only, no button ordering
- **Taskbar Thumbnail Reorder**: Only affects thumbnails, not main buttons
- **Taskbar Labels**: Text customization, no ordering
- **Disable Grouping**: Prevents grouping but doesn't reorder

**Conclusion**: No existing mod provides the functionality we need. Custom mod required.

### Windhawk API Functions Available

- `Wh_SetFunctionHook` - Hook into functions in target process
- `Wh_ModInit` / `Wh_ModUninit` - Lifecycle callbacks
- `Wh_Log` - Debugging/logging
- `Wh_GetIntSetting` / `Wh_GetStringSetting` - Settings management

### Windows Taskbar Structure

- **Main Window**: `Shell_TrayWnd` class
- **Toolbar Control**: `ToolbarWindow32` class (child of Shell_TrayWnd)
- **Button Messages**: 
  - `TB_BUTTONCOUNT` - Get button count
  - `TB_GETBUTTON` - Get button info
  - `TB_MOVEBUTTON` - Move button (may not work directly)
  - `TB_INSERTBUTTON` - Insert button
  - `TB_DELETEBUTTON` - Delete button

### Task Manager Detection

- **Window Class**: `TaskManagerWindow` (Windows 11)
- **Process Name**: `Taskmgr.exe`
- **Detection Methods**:
  1. Check window class name
  2. Check process name via `GetWindowThreadProcessId` + `QueryFullProcessImageName`

### Approaches to Consider

#### Approach 1: Hook Taskbar Button Creation
- **Target**: Function in explorer.exe that adds buttons to taskbar
- **Method**: Hook internal function, detect Task Manager, move to end
- **Status**: Need to identify function name via reverse engineering

#### Approach 2: Hook Window Creation
- **Target**: `CreateWindowExW` in explorer.exe
- **Method**: Detect Task Manager window creation, trigger reorder
- **Status**: Implemented as placeholder in mod skeleton
- **Limitation**: May not catch all cases, timing issues

#### Approach 3: Subclass Taskbar Window
- **Target**: `Shell_TrayWnd` window procedure
- **Method**: Intercept window messages, respond to button-related messages
- **Status**: Not yet implemented
- **Advantage**: Can catch all taskbar messages

#### Approach 4: Hook Taskbar Update/Refresh
- **Target**: Internal taskbar refresh function
- **Method**: Hook function that updates taskbar, force Task Manager to end
- **Status**: Need to identify function name

### Internal Explorer.exe Functions (To Be Identified)

These functions likely exist but need reverse engineering to find:

1. **Taskbar Button Management**:
   - Function that adds new buttons
   - Function that reorders buttons
   - Function that updates taskbar layout

2. **Possible Class Names** (from Windows internals research):
   - `CTaskListWnd` - Task list window class
   - `CTaskBand` - Task band class
   - `CTaskBtnGroup` - Task button group class

3. **Possible Function Patterns**:
   - `*AddTab*` - Add tab/button
   - `*Reorder*` - Reorder buttons
   - `*Update*` - Update taskbar
   - `*Refresh*` - Refresh taskbar

### Testing Strategy

1. **Use test-hook-explorer.wh.cpp**:
   - Load in Windhawk
   - Enable logging
   - Open Task Manager
   - Check logs for function calls and messages
   - Identify what gets called when Task Manager opens

2. **Use test scripts**:
   - `test-enumerate-taskbar.ps1` - Understand toolbar structure
   - `test-find-taskmanager.ps1` - Verify Task Manager detection
   - `test-taskbar-buttons.cpp` - Enumerate buttons programmatically

3. **Reverse Engineering Tools**:
   - IDA Pro / Ghidra - Static analysis of explorer.exe
   - x64dbg - Dynamic debugging
   - Process Monitor - Monitor API calls
   - API Monitor - Monitor specific API calls

### Next Steps

1. ✅ Create mod skeleton
2. ✅ Create test scripts
3. 🔄 Run test-hook-explorer mod to identify functions
4. ⏳ Reverse engineer explorer.exe to find function names
5. ⏳ Implement proper hooks based on findings
6. ⏳ Test with two-line taskbar
7. ⏳ Polish and release

### Resources

- [Windhawk Mod Development Guide](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windhawk Development Tips](https://github.com/ramensoftware/windhawk/wiki/Development-tips)
- [Windhawk Mods Repository](https://github.com/ramensoftware/windhawk-mods)
- Windows API: `TB_MOVEBUTTON`, `FindWindow`, `EnumChildWindows`, `SetWindowLongPtr`
