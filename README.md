# Windhawk Mod Lab

Development home for sb4ssman's [Windhawk](https://windhawk.net) mods.

These mods mostly explore dense Windows 11 taskbar and system tray layouts, especially
double-height taskbars with room for two-row tray controls.

## Mods

| Folder | Status | Description |
|--------|--------|-------------|
| [omnibutton-customizer/](omnibutton-customizer/) | v1.0, in development | Configurable grid layout for the Windows 11 OmniButton: wifi, volume, battery, and battery percentage |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | v0.9, in development | Keeps location, microphone, camera, and Copilot status placeholders visible in the tray to prevent layout shifts |
| [system-tray-grid-lines/](system-tray-grid-lines/) | concept | Notes for user-controlled visual grid lines between tray sections |
| [taskbar-clock-customization-spacer/](taskbar-clock-customization-spacer/) | submitted integration scratch | Elastic spacer support for Taskbar Clock Customization format strings |
| [taskbar-folder-menus/](taskbar-folder-menus/) | v0.5, submission prep | Compact taskbar buttons that open configured Shell targets as popup menus with right-click context menus and subfolder navigation |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published | Keeps Task Manager pinned to the end of the taskbar on Windows 10 and 11 |
| [taskbar-vd-switcher/](taskbar-vd-switcher/) | v1.6, published | Clickable virtual desktop buttons with grid layouts, Task View button, tray placement, and Start-adjacent placement |

## Repository Layout

```text
Windhawk-Mod-Lab/
  omnibutton-customizer/
  privacy-indicator-anchor/
  system-tray-grid-lines/
  taskbar-clock-customization-spacer/
  taskbar-folder-menus/
  taskmanager-tail/
  taskbar-vd-switcher/
  _archive/          old retired folders or moved work
  _claude_notes/     root-level development notes for all mods
  _profiles/         local Windhawk profile snapshots
  _research/         shared investigations and design notes
```

Each mod folder should have its own user-facing `README.md` and can keep its own
`archive/` folder for old implementation experiments. Development notes that are
only for Codex/Claude belong in root `_claude_notes/`, not inside individual mod
folders.
