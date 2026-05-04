# Taskbar Folder Menu

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
Desktop=%USERPROFILE%\Desktop
Downloads=%USERPROFILE%\Downloads
Tools=C:\Tools
```

Use one entry per line in the `Folders` setting. `|` also works as a separator for quick testing. Labels of one or two characters are shown on the button; longer labels use the default chevron text and remain visible in the tooltip.

## Design Notes

This starts with the conservative tray-column injection pattern used by the other taskbar mods in this repo. A likely next experiment is a placement mode that lines the folder buttons up directly beside the taskbar overflow button, if the live XAML tree gives us a stable named parent there.
