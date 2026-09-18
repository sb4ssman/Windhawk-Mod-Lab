# Windhawk Mod Lab

Development home for sb4ssman's [Windhawk](https://windhawk.net) mods.

These mods mostly explore dense Windows 11 taskbar and system tray layouts, especially
double-height taskbars with room for two-row tray controls.

## Mods

| Folder | Status | Description |
|--------|--------|-------------|
| [omnibutton-customizer/](omnibutton-customizer/) | v2.0, PR #4855 open; live-tested on 26200.9457 | Arrange the Windows 11 OmniButton's network, volume, battery, and percentage with one nestable layout expression, per-item color and opacity, and percentage size/font controls |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | v2.0, PR #4843 open; live-tested on 26200.9457 | Keeps location, microphone, camera, and Copilot status placeholders stable in the tray or beside Start, arranged with one nestable layout expression |
| [system-tray-grid-lines/](system-tray-grid-lines/) | concept | Notes for user-controlled visual grid lines between tray sections |
| [taskbar-clock-spacer/](taskbar-clock-spacer/) | v1.1, PR #4443 open; live-tested on 26200.9457 | Standalone companion mod adding elastic spacer tokens to Taskbar Clock Customization format strings |
| [taskbar-folder-menus/](taskbar-folder-menus/) | v0.7 published; v2.0 live-tested on 26200.9457, update PR pending | Grouped Shell-menu buttons with native icons, nested layouts and placement after app icons or in the tray |
| [taskmanager-tail/](taskmanager-tail/) | v1.1, published | Keeps Task Manager pinned to the end of the taskbar on Windows 10 and 11 |
| [taskbar-vd-switcher/](taskbar-vd-switcher/) | v1.7 published; PR #4844 open; v2.0 hover previews live-tested on 26200.9457 | Clickable desktop buttons with hover previews, customizable indicators, nested layouts, Task View button and tray/Start placement |
| [tray-utility-customizer/](tray-utility-customizer/) | v1.1 published; v2.0 on the shared contract, live-tested on 26200.9457 | Granular per-icon layout of the Windows 11 tray utilities (hidden icons, Emoji, touch keyboard, pen, touchpad) via one `Layout.Arrangement` expression |

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
