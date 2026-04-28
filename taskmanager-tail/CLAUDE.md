# CLAUDE.md — Task Manager Tail

## Notes

See [_claude_notes/Claude_Notes.md](_claude_notes/Claude_Notes.md) for architecture and current status.
See [_claude_notes/work_log.md](_claude_notes/work_log.md) for version history.
See [RESEARCH.md](RESEARCH.md) for Windows 10 event detection research.

## The mod

Single file: [task-manager-tail.wh.cpp](task-manager-tail.wh.cpp)

Shows CPU/RAM usage on the taskbar, tailing Task Manager data. Windows 11 only (v1.0 published).
v1.1-test adds Windows 10 support but is unfinished — event detection for app close/reorder is incomplete.

## Status: v1.0 published. v1.1 (Win10 support) stalled on event detection.

## Key open problem (v1.1)

UIA `StructureChangedEventHandler` only fires for `ChildAdded` — not `ChildRemoved` or reorder.
Options: hybrid UIA + `WinEventHook`, or `RegisterShellHookWindow`. See `_claude_notes/Claude_Notes.md`.
