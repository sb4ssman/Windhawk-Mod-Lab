# Working Notes — Windhawk Mod Lab

## Current focus

1. **Virtual Desktop Switcher** — v0.1 draft implementation complete; refining notification logic.
2. **Repo consolidation** — bring existing mod repos into this lab as subfolders with full git history

---

## Repo consolidation — DONE (2026-04-27)

Both existing mod repos subtree-merged with full history:
- `vertical-omnibutton/` ← Windhawk-Vertical-OmniButton
- `taskmanager-tail/` ← Windhawk-Taskmanager-Tail

Notes: `CLAUDE.md` and `_claude_notes/` were gitignored in the OmniButton source repo — copied in manually.
Original repos left as-is on GitHub (no need to touch them).

---

## Virtual Desktop Switcher status

Design doc: virtual-desktop-switcher-design.md  
Target location: `virtual-desktop-switcher/virtual-desktop-switcher.wh.cpp`

Implementation order (from design doc):
1. [DONE] Boilerplate + GetTaskbarXamlRoot (adapted from vertical-omnibutton v1.51.0)
2. [DONE] Desktop state: registry read + notification registration  
3. [DONE] SwitchToDesktop(int index): adapt from taskbar-empty-space-clicks
4. [DONE] RebuildButtons(): WinRT Button elements in a StackPanel
5. [DONE] Injection: find SystemTrayFrameGrid, InsertAt position
6. [DONE] Live update: Added Created/Destroyed listeners to notification object
7. [DONE] Cleanup: uninit removes bar, unregisters notifications
8. [DONE] Settings: position, size, colors, label format

**Next: test in Windhawk. Use testing sequence in CLAUDE.md.**

---

## TODO (future)

- **windhawk-mods PR update script** — a script that:
  1. Pulls latest from `ramensoftware/windhawk-mods` upstream into the local fork
  2. Copies the updated `.wh.cpp` from the lab mod folder into `mods/`
  3. Creates or updates a PR on the fork with appropriate commit message
  - Should handle both new mod submissions and version bumps to existing ones
  - The windhawk-mods fork lives at `t:/Github/sb4ssman/windhawk-mods/`

## Completed

- vertical-omnibutton v1.51.0 published (final calibration)
- taskmanager-tail v1.0 published
