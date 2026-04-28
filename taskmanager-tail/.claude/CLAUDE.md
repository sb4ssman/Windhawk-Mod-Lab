# Task Manager Tail - Project Memory

## Claude's Workflow

**Before every interaction:**
1. Read `_claude_notes/Claude_Notes.md` for current state and priorities
2. Check `RESEARCH.md` for technical context and findings
3. Reference `_claude_notes/work_log.md` for session history if needed

**During work:**
- This is a Windhawk mod written in C++
- When encountering issues, add them to the TODO section in Claude_Notes.md
- Don't interrupt workflow - capture items for later

**After completing work:**
1. Update `_claude_notes/Claude_Notes.md` with accomplishments
2. Add entry to `_claude_notes/work_log.md` with session summary
3. Update `RESEARCH.md` if technical findings were made

---

## Project Overview

### What is Task Manager Tail?
A Windhawk mod that automatically keeps Task Manager (or other configured apps) at the tail end of the Windows taskbar. When you open or close applications, the mod detects the change and moves the target button to the end.

**Primary Purpose:** Taskbar button position management

### Technical Stack
- **Language:** C++
- **Platform:** Windhawk (Windows customization framework)
- **Target:** Windows 10 and Windows 11
- **APIs Used:**
  - UI Automation (IUIAutomation)
  - ITaskbarList (shell interface)
  - WinEventHook (event detection)

### Core Architecture
- **Event Detection:** WinEventHook for window create/destroy events
- **Button Identification:** UI Automation to scan taskbar structure
- **Button Movement:** ITaskbarList::DeleteTab/AddTab to reposition
- **Debouncing:** Timer-based to coalesce rapid events

---

## Important Files

### Main Source Files
- **[task-manager-tail.wh.cpp](../task-manager-tail.wh.cpp)** - Production version (Windows 11 only, v1.0)
- **[test-task-manager-tail-with-win-10.cpp](../test-task-manager-tail-with-win-10.cpp)** - Test version with Windows 10 support

### Documentation
- **[README.md](../README.md)** - User-facing documentation
- **[RESEARCH.md](../RESEARCH.md)** - Technical findings and approach history
- **[QUICK-START.md](../QUICK-START.md)** - Quick installation guide

### Archive
- **[Archive/](../Archive/)** - Previous versions and experimental code

### Claude's Files
- **[_claude_notes/Claude_Notes.md](../_claude_notes/Claude_Notes.md)** - Current session notes and TODOs
- **[_claude_notes/work_log.md](../_claude_notes/work_log.md)** - Session history
- **[_claude_notes/Offline_Notes.md](../_claude_notes/Offline_Notes.md)** - User's notes to Claude between sessions

---

## Windows Version Differences

### Windows 11 Taskbar
- Uses XAML Islands (`DesktopWindowXamlSource`)
- Button class: `TaskListButtonAutomationPeer`
- WinEventHook works well with minimal noise

### Windows 10 Taskbar
- Traditional Win32 controls
- Hierarchy: `Shell_TrayWnd → ReBarWindow32 → MSTaskSwWClass → MSTaskListWClass`
- Button type: `UIA_ButtonControlTypeId`
- WinEventHook generates excessive noise - requires different approach

---

## Code Guidelines

### File References
Use markdown links with line numbers:
- File: [task-manager-tail.wh.cpp](../task-manager-tail.wh.cpp)
- Line: [task-manager-tail.wh.cpp:122](../task-manager-tail.wh.cpp#L122)

### Development Principles
- Read before modifying - Always read files before editing
- Prefer editing over creating - Don't create new files unnecessarily
- Keep solutions simple - Minimum complexity for current task
- Test on target OS - Behavior differs between Win10/Win11

### Windhawk Specifics
- Mod runs in dedicated `windhawk.exe` process (not injected into explorer)
- Uses `Wh_Log()` for debug output (visible in DbgView)
- Settings defined in `// ==WindhawkModSettings==` block
- Compiler options in `// @compilerOptions` directive

---

## Testing

### Debug Output
Use DbgViewMini or similar to capture `OutputDebugString` messages:
- Filter by `windhawk.exe` or mod ID
- Look for version detection, event triggers, and move operations

### Test Scenarios
1. Open new app → target should move to end
2. Close app → target should remain at end (or move if displaced)
3. Manual drag → target should return to end on next event
4. Stress test → open many apps rapidly, target should end up at tail

---

## Version History

### v1.0 - Windows 11 Support
- Event-driven detection via WinEventHook
- UI Automation for button enumeration
- ITaskbarList for button repositioning
- Debouncing to handle rapid events

### v1.1 - Windows 10 Support (In Development)
- Added version detection (explorer.exe file version)
- Win10: Different taskbar hierarchy (MSTaskListWClass)
- Win10: UIA StructureChangedEventHandler approach (testing)
- Fallback to WinEventHook if UIA handler fails

---

## Remember

**Your notes are your memory between sessions.**

Keep `_claude_notes/Claude_Notes.md` and `_claude_notes/work_log.md` current. When in doubt about context - **READ YOUR NOTES FIRST**.
