# Task Manager Tail - Work Log

Permanent record of development sessions and changes.

---

## Pre-History: v1.0 Development (Windows 11)

Based on [RESEARCH.md](../RESEARCH.md) and git history, the v1.0 development included:

### Technical Approaches Explored

1. **Polling (Timer-based)** - REJECTED
   - Method: Check button positions every 50-100ms
   - Problem: Wastes CPU, causes visual lag

2. **UI Automation Events (inside explorer.exe)** - REJECTED
   - Method: Register `IUIAutomationStructureChangedEventHandler` within explorer
   - Problem: Caused deadlocks and UI freezes

3. **WinEventHook (Global Hooks)** - CHOSEN for v1.0
   - Method: `SetWinEventHook` for `EVENT_OBJECT_CREATE/HIDE`
   - Result: Lightweight, event-driven, works well on Windows 11

### v1.0 Architecture
- **Detection:** WinEventHook for window create/hide events
- **Debouncing:** 300ms timer to coalesce rapid events
- **Identification:** UI Automation scans `DesktopWindowXamlSource` XAML Island
- **Action:** `ITaskbarList::DeleteTab/AddTab` to move buttons
- **Configuration:** Target name (partial match) and window class

### v1.0 Limitations
- Windows 11 only
- Single target application
- No multi-app sorting support

---

## 2026-01-26: Windows 10 Support Development (v1.1)

### Session Goal
Add Windows 10 support while maintaining Windows 11 functionality.

### Research Phase
- Explored Windhawk documentation for version detection capabilities
- Found: Windhawk doesn't provide built-in version API
- Solution: Check `explorer.exe` file version (build < 22000 = Win10)
- Researched Win10 vs Win11 taskbar architecture differences

### Key Findings: Taskbar Differences

| Aspect | Windows 11 | Windows 10 |
|--------|-----------|------------|
| Container | `DesktopWindowXamlSource` (XAML Island) | `MSTaskListWClass` (Win32 toolbar) |
| Button class | `TaskListButtonAutomationPeer` | `UIA_ButtonControlTypeId` |
| Hierarchy | Flat XAML tree | `Shell_TrayWnd → ReBarWindow32 → MSTaskSwWClass → MSTaskListWClass` |

### Implementation Attempt #1: WinEventHook (Same as Win11)

**File:** `test-task-manager-tail-with-win-10.cpp` (initial version, archived as `win-10-test-1.cpp`)

**Changes:**
- Added `WinVersion` enum and `DetectWindowsVersion()` function
- Added `CheckAndMoveWin10()` for Win10 taskbar hierarchy
- Modified `CheckAndMove()` to dispatch by version

**Result:** Partially successful
- Button movement works correctly
- **Problem:** Excessive event noise on Win10
- WinEventHook fires for tooltips, animations, unrelated UI events
- Check function runs every few seconds even when idle
- Floods logs, wastes CPU

### Implementation Attempt #2: UIA StructureChangedEventHandler

**File:** `test-task-manager-tail-with-win-10.cpp` (current version)

**Rationale:**
- Original research rejected UIA events due to deadlocks *inside explorer.exe*
- But we run in separate windhawk.exe process now
- UIA events on specific element should only fire when that element changes

**Changes:**
- Added `TaskbarStructureChangedHandler` class implementing `IUIAutomationStructureChangedEventHandler`
- Register handler on `MSTaskListWClass` element
- Handler posts `WM_TRIGGER_CHECK` when structure changes
- Falls back to WinEventHook if registration fails
- Windows 11 continues using WinEventHook (works well there)

**Result:** Partial success
- Much quieter than WinEventHook approach
- Events fire for `ChildAdded` (new apps opening) ✓
- Events do NOT fire for `ChildRemoved` (apps closing) ✗
- Events do NOT fire for `ChildrenReordered` (manual drag) ✗

**Conclusion:** Win10's taskbar doesn't properly fire all UIA structure events. May need hybrid approach.

### Documentation Updates
- Updated `RESEARCH.md` with Windows 10 findings
- Created new section documenting both attempts and results

### Files Created/Modified
- `test-task-manager-tail-with-win-10.cpp` - Win10+11 test version
- `win-10-test-1.cpp` - Archived first attempt (moved to Archive/)
- `RESEARCH.md` - Added Win10 support section

### Open Issues
- Win10 event detection incomplete (only catches new app opens)
- Need to decide: hybrid approach vs accept limitation vs try shell hooks

---

## 2026-01-26: Claude Notes Cleanup

### Session Goal
Clean up `.claude/` and `_claude_notes/` folders copied from another project (Windopener).

### Changes Made
- Rewrote `.claude/CLAUDE.md` for Task Manager Tail project
- Rewrote `_claude_notes/Claude_Notes.md` with current TODOs and status
- Rewrote `_claude_notes/work_log.md` with project history (this file)
- Updated `_claude_notes/Offline_Notes.md` (cleared)
- Updated `_claude_notes/claude_tools/generate_folder_map.py` (made project-agnostic)
- Regenerated `_claude_notes/_claude_outputs/folder_structure.md`

---

## Future Work

### Potential v1.1 Improvements
1. **Hybrid event detection for Win10**
   - Keep UIA handler for `ChildAdded`
   - Add narrow WinEventHook for `EVENT_OBJECT_DESTROY`

2. **Shell hooks alternative**
   - `RegisterShellHookWindow` for `HSHELL_WINDOWCREATED/DESTROYED`
   - More targeted than broad WinEventHook

3. **Accept limitation**
   - Document that Win10 only triggers on new app opens
   - Target eventually moves to end as apps are opened
