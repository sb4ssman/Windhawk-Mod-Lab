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

## Phase 2: Execution & Findings

### 1. Legacy Methods (Failed)
- **Attempt**: Used `TB_BUTTONCOUNT` and `TB_MOVEBUTTON` on `ToolbarWindow32`.
- **Result**: Returned 0 buttons. Windows 11 uses XAML Islands (`DesktopWindowXamlSource`), rendering legacy toolbar messages useless.

### 2. UI Automation (UIA) (Partial Success)
- **Discovery**: We can successfully find the "Task Manager" button using `IUIAutomation`.
- **Capability**: The button supports the `DragPattern`, confirming it is draggable.
- **Prototype v7.3**: We implemented a background thread that detects when Task Manager is out of place and uses `SendInput` to physically drag the mouse to move it.
- **Verdict**: **Rejected**. While functional, hijacking the user's mouse cursor is a poor user experience.

### 3. Native Hooks (v8.0) (Failed)
- **Attempt**: Hooked `DoDragDrop` in `ole32.dll` and manually dragged taskbar buttons.
- **Result**: No calls were intercepted.
- **Conclusion**: The Windows 11 XAML Taskbar handles drag-and-drop internally (likely via `Windows.UI.Xaml.dll` input processing) and does not use the standard OLE Drag/Drop subsystem.

### 4. Mouse Simulation (v9.x) (Rejected)
- **Attempt**: Implemented various versions of mouse simulation (Slow, Instant, Precision).
- **Result**: While technically functional, it inherently conflicts with user input ("fighting" for the cursor) and is unreliable (timing issues with drop targets).
- **Verdict**: **Abandoned**. A solution that hijacks the mouse is unacceptable.

### 5. Deep API Inspection (v10.0) (Findings)
- **Scan Results**:
  - Button supports: `Invoke`, `ScrollItem`, `LegacyIAccessible`, `Drag`.
  - Parent supports: `ScrollItem`, `LegacyIAccessible`, `Drag`.
  - **Missing**: `Transform` (Move) pattern is NOT supported.
- **Conclusion**: Standard UIA does not expose a programmatic "Move" method.

### 6. Virtual Input (v10.1) (Failed)
- **Attempt**: Sent `WM_LBUTTONDOWN` etc. to the window handle.
- **Result**: Messages sent, but no movement occurred. XAML Islands likely ignore legacy mouse messages injected this way or require specific pointer flags.

### 7. Touch Injection (v11.0) (Rejected)
- **Verdict**: Rejected by user. Simulating input (even touch) is risky and potentially intrusive ("fighting" the user).

### 8. Current Strategy: Keyboard Accessibility (v12.0)
- **Discovery**: Windows 11 supports `Alt` + `Shift` + `Arrow Keys` to reorder focused taskbar items.
- **Method**:
  1. Use UIA to `SetFocus()` on the Task Manager button.
  2. Send synthetic KeyDown/KeyUp events for `Alt+Shift+Right`.
  3. Restore focus to the user's previous window.
- **Advantage**: Uses built-in accessibility features. No mouse movement. No coordinate math.
- **Result**: Failed. `SetFocus` worked, but the keystrokes did not move the button.

### 9. Current Strategy: Window State Cycling (v15.0)
- **Goal**: Force the Taskbar to re-evaluate the button position by removing and re-adding it.
- **Method**: Use `ShowWindow(SW_HIDE)` followed by `ShowWindow(SW_SHOW)` on the Task Manager window itself.
- **Hypothesis**: When a window is shown, its taskbar button is usually appended to the end of the list (or group). This leverages standard Windows behavior rather than hacking the input stream.
- **Result**: Failed. `SW_HIDE` was blocked by UIPI (User Interface Privilege Isolation) because Task Manager runs as High Integrity (Admin) and Explorer (User) cannot send it hide messages.

### 10. Current Strategy: ITaskbarList Interface (v16.0)
- **Goal**: Use the official Windows API for managing taskbar buttons.
- **Method**: Instantiate `ITaskbarList` (or `ITaskbarList2/3/4`) and call `DeleteTab()` then `AddTab()` on the Task Manager window handle.
- **Hypothesis**: Since we are running inside `explorer.exe`, we might have privileges or direct access to manipulate the list via this interface, effectively "editing the document" as requested.
- **Result**: **Success!** v16.0 worked smoothly. v16.1 failed due to incorrect HWND retrieval from UIA elements. v16.2 reverts to v16.0 logic with settings.

### Resources

- [Windhawk Mod Development Guide](https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod)
- [Windhawk Development Tips](https://github.com/ramensoftware/windhawk/wiki/Development-tips)
- [Windhawk Mods Repository](https://github.com/ramensoftware/windhawk-mods)
- Windows API: `TB_MOVEBUTTON`, `FindWindow`, `EnumChildWindows`, `SetWindowLongPtr`
