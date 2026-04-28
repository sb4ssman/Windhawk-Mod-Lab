# Task Manager Tail - Claude's Working Notes

**Last Updated:** 2026-01-26

---

## Current Status

**Production Version:** v1.0 - Windows 11 only ([task-manager-tail.wh.cpp](../task-manager-tail.wh.cpp))

**Development Version:** v1.1-test - Windows 10 + 11 ([test-task-manager-tail-with-win-10.cpp](../test-task-manager-tail-with-win-10.cpp))

---

## TODO List

### HIGH PRIORITY

- [ ] **Windows 10 event detection incomplete**
  - UIA StructureChangedEventHandler only fires for `ChildAdded` (new apps opening)
  - Does NOT fire for `ChildRemoved` (apps closing) or `ChildrenReordered` (manual drag)
  - May need hybrid approach: UIA for adds + WinEventHook for destroys
  - See [RESEARCH.md](../RESEARCH.md) for details

### MEDIUM PRIORITY

- [ ] **Finalize v1.1 for Windows 10**
  - Decide on event detection strategy
  - Test thoroughly on Win10
  - Update README with Win10 support notes

- [ ] **Consider alternative Win10 approaches**
  - Shell hooks (`RegisterShellHookWindow`)
  - Filtered WinEventHook (only `EVENT_OBJECT_DESTROY`)
  - Hybrid: UIA + narrow WinEventHook

### LOW PRIORITY

- [ ] **Code cleanup**
  - Remove excessive logging from production build
  - Consider combining Win10/Win11 into single production file

---

## Windows 10 Testing Observations (2026-01-26)

### What Works
- Version detection correctly identifies Win10 (build < 22000)
- Taskbar hierarchy navigation works (`MSTaskListWClass` found)
- UIA StructureChangedEventHandler registers successfully
- Button movement via ITaskbarList works correctly
- Opening new apps triggers move correctly

### What Doesn't Work
- Closing apps does NOT trigger event
- Manual drag does NOT trigger event
- Events only fire for `ChildAdded`, not `ChildRemoved` or `ChildrenReordered`

### Conclusion
Win10's taskbar doesn't fire all expected UIA structure events. Need hybrid approach or alternative event source.

---

## Quick Reference

- **Entry point:** `Wh_ModInit()` in `.wh.cpp` file
- **Background thread:** `BackgroundThread()` - main event loop
- **Win10 check:** `CheckAndMoveWin10()` - taskbar scanning
- **Win11 check:** `CheckAndMoveWin11()` - XAML Island scanning
- **Move action:** `CycleTaskbarTab()` - ITaskbarList operations
- **Version detect:** `DetectWindowsVersion()` - explorer.exe file version

---

## Files Structure

```
Windhawk-Task-Manager-Tail/
├── task-manager-tail.wh.cpp      # Production v1.0 (Win11)
├── test-task-manager-tail-with-win-10.cpp  # Test v1.1 (Win10+11)
├── README.md                     # User documentation
├── RESEARCH.md                   # Technical findings
├── QUICK-START.md               # Installation guide
├── Archive/                      # Old versions
│   ├── win-10-test-1.cpp        # First Win10 attempt (WinEventHook)
│   └── ...
├── .claude/
│   └── CLAUDE.md                # Project memory for Claude
└── _claude_notes/
    ├── Claude_Notes.md          # THIS FILE
    ├── work_log.md              # Session history
    ├── Offline_Notes.md         # User notes to Claude
    ├── _claude_outputs/
    │   └── folder_structure.md  # Auto-generated map
    └── claude_tools/
        └── generate_folder_map.py
```

---

## Session Notes: 2026-01-26

### What We Did
1. Explored project from .md files and .cpp source
2. Researched Windhawk documentation and Windows version detection
3. Implemented Windows 10 support with version branching
4. **First attempt:** WinEventHook (same as Win11)
   - Result: Works but excessive event noise on Win10
5. **Second attempt:** UIA StructureChangedEventHandler
   - Result: Quieter but incomplete event coverage
6. Updated RESEARCH.md with findings
7. Cleaned up `.claude/` and `_claude_notes/` folders (removed Windopener content)

### Files Modified
- `test-task-manager-tail-with-win-10.cpp` - Added Win10 support
- `RESEARCH.md` - Added Win10 findings
- `.claude/CLAUDE.md` - Rewritten for this project
- `_claude_notes/Claude_Notes.md` - Rewritten for this project
- `_claude_notes/work_log.md` - Added session history
