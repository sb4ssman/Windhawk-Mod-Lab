# Taskbar Folder Menus

A Windhawk mod that adds compact taskbar buttons for opening configured folders as popup menus.

The first target is the classic Desktop toolbar workflow: click a tiny taskbar button and open something from the Desktop folder without minimizing the window currently covering it.

## Current Behavior

- Injects into the Windows 11 system tray grid.
- Supports one or more folder buttons.
- Opens a native popup menu with folder contents.
- Opens files, shortcuts, and max-depth folders with `ShellExecute`.
- Supports nested folder submenus up to a configurable depth.
- Can arrange buttons in a row, column, or grid.

## Settings Example

```text
📄=%DOCUMENTS%
📁=%DESKTOP%
C:=C:\
T:=T:\
```

The `Folders` setting is a list of entries in `Label=Path` form. Separate entries with newlines, `|`, or commas, so `📄=%DOCUMENTS%,📁=%DESKTOP%,C:=C:\` is valid.

Compact labels work best. One- or two-character labels are shown on the button. Longer text labels can be clipped by the configured button size; the full label and path remain visible in the tooltip.

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

`%DESKTOP%` points to the user's Desktop folder. It may differ from the actual desktop shell view, which can include virtual items such as This PC, Network, Recycle Bin, and special shortcuts.

## Theme Matching

By default, buttons use the normal Windows 11 taskbar button styling. Optional color and shape settings can override text, background, hover background, click background, border color, border thickness, corner rounding, and label font size. Leave color fields empty and numeric shape settings at `-1` to keep the system default.

## Design Notes

This starts with the conservative tray-column injection pattern used by the other taskbar mods in this repo. A likely next experiment is a placement mode that shares or replaces the show-hidden-icons chevron area with a compact two- or three-button column, if the live XAML tree gives us a stable named parent there.
