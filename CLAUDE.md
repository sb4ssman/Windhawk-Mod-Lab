# CLAUDE.md - Windhawk Mod Lab

## First: read the notes

Go to [_claude_notes/](_claude_notes/) and read:

1. [working_notes.md](_claude_notes/working_notes.md) - current goals, active work, key facts
2. [work_log.md](_claude_notes/work_log.md) - completed lab-level work log

Use the mod-prefixed files in `_claude_notes/` for older per-mod context. Do not
create new per-mod Claude folders. If a mod needs development notes, put them in
root `_claude_notes/` with a clear mod prefix.

Also check [_research/](_research/) for investigations, design docs, and open
questions on specific problems.

## README policy

When a mod's status changes, update [README.md](README.md). Each mod folder
should also have its own detailed user-facing `README.md`.

## Repo shape

Each mod lives in its own lowercase subdirectory. A mod folder may have its own
`archive/` folder for old code experiments, but development-agent notes belong
at the repo root in `_claude_notes/`.

| Folder | Status |
|--------|--------|
| [omnibutton-customizer/](omnibutton-customizer/) | v1.0, in development |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | v0.9, in development |
| [system-tray-grid-lines/](system-tray-grid-lines/) | concept |
| [taskbar-clock-customization-spacer/](taskbar-clock-customization-spacer/) | PR-ready integration scratch |
| [taskbar-folder-menus/](taskbar-folder-menus/) | v0.5, submission prep |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published |
| [virtual-desktop-switcher/](virtual-desktop-switcher/) | v1.4, submission prep |

## Versioning policy

- Major: architecture changes or complete rewrites.
- Minor: new features or significant behavior changes.
- Patch: bug fixes and verified internal lab calibrations.

## Windhawk mod basics

- `Wh_ModInit` - called when the mod loads into the target process
- `Wh_ModAfterInit` - called after hooks are applied
- `Wh_ModUninit` - called when the mod unloads; must restore state
- `Wh_ModSettingsChanged` - called when the user saves settings in Windhawk
- `Wh_GetIntSetting` / `Wh_GetStringSetting` / `Wh_FreeStringSetting` - settings access
- `Wh_Log(L"...")` - debug logging
- Settings are declared in the `// ==WindhawkModSettings==` YAML block

## Common patterns across taskbar mods

- `GetTaskbarXamlRoot(HWND)` - hooks `taskbar.dll` symbols to get the XAML root
- `RunFromWindowThread(HWND, proc, param)` - marshal work to the UI thread with a `WH_CALLWNDPROC` hook
- `FindCurrentProcessTaskbarWnd()` - finds `Shell_TrayWnd` for the current process
- `FindChildRecursive(element, predicate)` - recursive XAML tree search
- System tray module discovery should prefer `SystemTray.dll`, fall back to older `Taskbar.View.dll` when appropriate, then `ExplorerExtensions.dll`
- Background retry threads must be stoppable and waited during `Wh_ModUninit`
