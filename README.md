# Windhawk Mod Lab

Development home for sb4ssman's [Windhawk](https://windhawk.net) mods.

These mods mostly explore dense Windows 11 taskbar and system tray layouts, especially
double-height taskbars with room for two-row tray controls.

## Mods

| Folder | Status | Description |
|--------|--------|-------------|
| [omnibutton-customizer/](omnibutton-customizer/) | v1.0, submitted (PR #3859), CI green | Per-item layout, visibility, color, size, font, opacity, and position control for the Windows 11 OmniButton |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | v1.0, submitted (PR open) | Keeps location, microphone, camera, and Copilot status placeholders stable in the tray or beside Start |
| [system-tray-grid-lines/](system-tray-grid-lines/) | concept | Notes for user-controlled visual grid lines between tray sections |
| [taskbar-clock-spacer/](taskbar-clock-spacer/) | v1.1, submitted (PR #4443) | Standalone companion mod adding elastic spacer tokens to Taskbar Clock Customization format strings |
| [taskbar-folder-menus/](taskbar-folder-menus/) | v0.7, submitted in PR #4485 | Compact taskbar buttons that open configured Shell targets as popup menus with classic Shell context menus and subfolder navigation |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published | Keeps Task Manager pinned to the end of the taskbar on Windows 10 and 11 |
| [taskbar-vd-switcher/](taskbar-vd-switcher/) | v1.8 submitted (PR #4844), CI green | Clickable virtual desktop buttons with customizable indicators, native checked states, grid layouts, Task View button, and tray/Start placement |
| [tray-utility-customizer/](tray-utility-customizer/) | v1.0, submitted (PR open) | Arranges detected Windows tray utilities into a configurable row, column, or smart grid |

## Repository Layout

```text
Windhawk-Mod-Lab/
  omnibutton-customizer/
  privacy-indicator-anchor/
  system-tray-grid-lines/
  taskbar-clock-spacer/
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
