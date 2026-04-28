# CLAUDE.md — Windhawk Mod Lab

## First: read the notes

Go to [_claude_notes/](_claude_notes/) and read:
1. [working_notes.md](_claude_notes/working_notes.md) — current goals, active work, key facts
2. [work_log.md](_claude_notes/work_log.md) — completed work log (per-mod sections)

Keep notes up to date. Keep working_notes short.

## Keep README.md updated

When a mod's status changes (new version, new mod added, mod archived), update [README.md](README.md).
The table at the top of README lists all mods with status, one-line description, and links.
Do this as part of any commit that changes a mod's published state.

## The lab

This is an umbrella repo for all of sb4ssman's Windhawk mods.

Each mod lives in its own subdirectory. Reference mods (community mods not authored here)
live in the sibling repo `t:\Github\sb4ssman\windhawk-mods\mods\`.

### Mods in this repo

| Folder | Status | Description |
|--------|--------|-------------|
| [vertical-omnibutton/](vertical-omnibutton/) | v1.51.0, published | Vertical stacking of system tray OmniButton (wifi/volume/battery) |
| [taskmanager-tail/](taskmanager-tail/) | v1.0, published | Task manager CPU/RAM tail on taskbar |
| [virtual-desktop-switcher/](virtual-desktop-switcher/) | v0.5, ready to submit | Virtual desktop switcher buttons injected into system tray |

### Mods to be brought in (separate repos → subfolders here)
- `t:\Github\sb4ssman\Windhawk-Vertical-OmniButton\` → `vertical-omnibutton/`
- `t:\Github\sb4ssman\Windhawk-Taskmanager-Tail\` → `taskmanager-tail/`

See [_claude_notes/working_notes.md](_claude_notes/working_notes.md) for consolidation plan.

## Versioning Policy

- **Major**: Architecture changes or complete rewrites.
- **Minor**: New features or significant logic updates.
- **Patch**: Bug fixes and internal "Lab" calibrations. Bump this for every verified fix.

## Windhawk mod basics

- `Wh_ModInit` — called when mod loads into explorer.exe
- `Wh_ModAfterInit` — called after hooks are applied
- `Wh_ModUninit` — called when mod unloads; must restore state
- `Wh_ModSettingsChanged` — called when user saves settings in Windhawk UI
- `Wh_GetIntSetting` / `Wh_GetStringSetting` / `Wh_FreeStringSetting` — settings access
- `Wh_Log(L"...")` — debug logging (shown in Windhawk log panel)
- Settings declared in `// ==WindhawkModSettings==` YAML block at top of file

## Key reference files (sibling repo)

- `taskbar-desktop-indicator.wh.cpp` — desktop detection, COM notification, registry reading
- `taskbar-empty-space-clicks.wh.cpp` — SwitchVirtualDesktop() at lines 2873–3073
- `taskbar-content-presenter-injector.wh.cpp` — XAML element injection pattern
- `windows-11-taskbar-styler.wh.cpp` — system tray XAML tree structure reference

## Common patterns across mods

- `GetTaskbarXamlRoot(HWND)` — hooks `taskbar.dll` symbols to get XAML root
- `RunFromWindowThread(HWND, proc, param)` — marshal to UI thread via WH_CALLWNDPROC hook
- `FindCurrentProcessTaskbarWnd()` — finds `Shell_TrayWnd` for current process
- `FindChildRecursive(element, predicate)` — recursive XAML tree search
- Background retry in `Wh_ModAfterInit` — 5× at 2s intervals if XAML tree not ready
- `@compilerOptions: -lole32 -loleaut32 -lruntimeobject`
