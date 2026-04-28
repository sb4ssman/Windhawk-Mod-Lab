# Working Notes — Windhawk Mod Lab

## Current focus

1. **VD Switcher** — new mod, design doc ready at [vd-switcher-design.md](vd-switcher-design.md)
2. **Repo consolidation** — bring existing mod repos into this lab as subfolders with full git history

---

## Repo consolidation — DONE (2026-04-27)

Both existing mod repos subtree-merged with full history:
- `vertical-omnibutton/` ← Windhawk-Vertical-OmniButton
- `taskmanager-tail/` ← Windhawk-Taskmanager-Tail

Notes: `CLAUDE.md` and `_claude_notes/` were gitignored in the OmniButton source repo — copied in manually.
Original repos left as-is on GitHub (no need to touch them).

---

## VD Switcher status

Design doc: [vd-switcher-design.md](vd-switcher-design.md)  
Target location: `vd-switcher/vd-switcher.wh.cpp`

Implementation order (from design doc):
1. Boilerplate + GetTaskbarXamlRoot (copy from vertical-omnibutton)
2. Desktop state: registry read + notification registration  
3. SwitchToDesktop(int index): adapt from taskbar-empty-space-clicks
4. RebuildButtons(): WinRT Button elements in a StackPanel
5. Injection: find SystemTrayFrameGrid, InsertAt position
6. Live update: notification → dispatch RebuildButtons on UI thread
7. Cleanup: uninit removes bar, unregisters notifications
8. Settings: position, size, colors, label format

**Not yet started** — discuss approach with user before writing code.

---

## Completed

- vertical-omnibutton v1.2 published to PR #3859 on ramensoftware/windhawk-mods
- taskmanager-tail v1.0 published
