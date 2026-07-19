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
| [taskbar-folder-menus/](taskbar-folder-menus/) | v0.6, PR #4485 revisions in local test | Compact taskbar buttons that open configured Shell targets as popup menus with classic Shell context menus and subfolder navigation |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published | Keeps Task Manager pinned to the end of the taskbar on Windows 10 and 11 |
| [taskbar-vd-switcher/](taskbar-vd-switcher/) | v1.7, local test; v1.6 update PR open | Clickable virtual desktop buttons with grid layouts, Task View button, tray placement, and Start-adjacent placement |
| [tray-utility-customizer/](tray-utility-customizer/) | v0.3, lab test | Arranges detected Windows tray utilities into a configurable row, column, or grid |

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
  tray-utility-customizer/
  .agents/           agent instructions, notes, tools, and generated outputs
  _archive/          old retired folders or moved work
  _profiles/         local Windhawk profile snapshots
  _research/         shared investigations and design notes
  _templates/        shared code templates and submission checklists
```

Each mod folder should have its own user-facing `README.md` and can keep its own
`archive/` folder for old implementation experiments. Development-agent notes
belong in `.agents/`, not inside individual mod folders; `CLAUDE.md` and
`AGENTS.md` are identical pointers into that folder.
