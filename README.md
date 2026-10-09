# Windhawk Mod Lab

[![Experimental Windows 10 VD Switcher: three desktops and Task View in a grid](taskbar-vd-switcher/assets/win10-experimental-grid.png)](taskbar-vd-switcher/)
*Experimental Windows 10 VD Switcher — live-tested compact grid on a single-height taskbar.*

[![Experimental Windows 10 VD Switcher: two desktops stacked before the hidden-icons chevron](taskbar-vd-switcher/assets/win10-experimental-stack.png)](taskbar-vd-switcher/)
*Experimental Windows 10 VD Switcher — two desktops stacked before the hidden-icons chevron.*

Development home for sb4ssman's [Windhawk](https://windhawk.net) mods.

All seven user-facing mods are published in the Windhawk catalogue (October 8, 2026).

These mods mostly explore dense Windows 11 taskbar and system tray layouts, especially
double-height taskbars with room for two-row tray controls.

## Mods

| Folder | Status | Description |
|--------|--------|-------------|
| [omnibutton-customizer/](omnibutton-customizer/) | [v2.1, published](https://windhawk.net/mods/omnibutton-customizer) | Arrange the Windows 11 OmniButton's network, volume, battery, and percentage with one nestable layout expression, per-item color and opacity, and percentage size/font controls |
| [privacy-indicator-anchor/](privacy-indicator-anchor/) | [v2.1, published](https://windhawk.net/mods/privacy-indicator-anchor) | Keeps location, microphone, camera, and Copilot status placeholders stable in the tray or beside Start, arranged with one nestable layout expression |
| [system-tray-grid-lines/](system-tray-grid-lines/) | concept | Notes for user-controlled visual grid lines between tray sections |
| [taskbar-clock-spacer/](taskbar-clock-spacer/) | [v1.1, published](https://windhawk.net/mods/taskbar-clock-spacer) | Standalone companion mod adding elastic spacer tokens to Taskbar Clock Customization format strings |
| [taskbar-folder-menus/](taskbar-folder-menus/) | [v2.1, published](https://windhawk.net/mods/taskbar-folder-menus) | Grouped Shell-menu buttons with native icons, nested layouts and placement after app icons or in the tray |
| [taskmanager-tail/](taskmanager-tail/) | [v1.1, published](https://windhawk.net/mods/task-manager-tail) | Keeps Task Manager pinned to the end of the taskbar on Windows 10 and 11 |
| [taskbar-vd-switcher/](taskbar-vd-switcher/) | [v2.1, published](https://windhawk.net/mods/taskbar-vd-switcher) | Desktop buttons with nested arrangements and hover previews; three experimental Windows 10 tray positions |
| [tray-utility-customizer/](tray-utility-customizer/) | [v2.1, published](https://windhawk.net/mods/tray-utility-customizer) | Granular per-icon layout of the Windows 11 tray utilities (hidden icons, Emoji, touch keyboard, pen, touchpad) via one `Layout.Arrangement` expression |

## Local experiments

[experiments/](experiments/) contains the confirmation-lights/DCG checker,
OmniButton/Quick Settings icon-hosting probe, and retro stats-dashboard prototype.
These are local experiments, not catalogue releases. Privacy Indicator Anchor
also has an **untested local candidate** adding optional Recall and OneDrive
process/policy indicators; the published version remains 2.1.

## Tools for mod authors

| Folder | Status | Description |
|--------|--------|-------------|
| [tool-mods/mod-tool-taskbar-tree-dump.wh.cpp](tool-mods/mod-tool-taskbar-tree-dump.wh.cpp) | v1.0, PR #5977 | Writes the Windows 11 taskbar's XAML element tree to a text or JSON file - on load, whenever the taskbar moves, resizes or is rebuilt, and on demand. Read-only |

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
  tool-mods/         Windhawk mods that are tools for mod authors, not end users
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
