# Taskbar Folder Menus

A Windhawk mod that adds compact taskbar buttons for opening configured Shell targets as popup menus.

The first target is the classic Desktop toolbar workflow: click a tiny taskbar button and open something from the Desktop, a drive root, or another Shell namespace without minimizing the window currently covering it.

## Current Behavior

- Injects into the Windows 11 system tray grid.
- Supports one or more folder buttons.
- Opens native popup menus backed by Shell PIDLs rather than raw filesystem enumeration.
- Shows Shell-provided small icons for menu items.
- Opens selected items with `ShellExecuteEx` by PIDL.
- Supports nested folder submenus up to a configurable depth.
- Can arrange buttons in a row, column, or grid.

## Baseline Test Settings

```text
T:=T:\
DSK=shell:Desktop
CTL=shell:ControlPanelFolder
```

The `Folders` setting is a list of entries in `Label=Target` form. Separate entries with newlines, `|`, or commas, so `T:=T:\,DSK=shell:Desktop,CTL=shell:ControlPanelFolder` is valid.

Compact labels work best. One-, two-, and three-character labels are shown on the button. Longer text labels fall back to the configured default icon text; the full label and target remain visible in the tooltip.

Label ideas:

```text
📁 folder
🖥 desktop PC
💻 laptop
🪟 desktop/window
📥 downloads
🌐 network
🗄 drive
📄 documents
🔧 tools
⚙ settings
⭐ favorites
```

`shell:Desktop` points at the actual Desktop Shell namespace, not only the user's physical Desktop folder. `shell:ControlPanelFolder` exercises a non-filesystem namespace and is useful for proving the PIDL path is working.

## Theme Matching

By default, buttons use the normal Windows 11 taskbar button styling. Optional color and shape settings can override text, background, hover background, click background, border color, border thickness, corner rounding, and label font size. Leave color fields empty and numeric shape settings at `-1` to keep the system default.

## Design Notes

This starts with the conservative tray-column injection pattern used by the other taskbar mods in this repo. A likely next experiment is a placement mode that shares or replaces the show-hidden-icons chevron area with a compact two- or three-button column, if the live XAML tree gives us a stable named parent there.
