# Vertical OmniButton — Windhawk Mod

[![Windhawk](https://img.shields.io/badge/Windhawk-Mod-blue)](https://windhawk.net/)
[![Windows 11](https://img.shields.io/badge/Windows-11-blue)](https://www.microsoft.com/windows/windows-11)

Rearranges the Windows 11 system tray OmniButton (wifi, volume/sound, battery) from
horizontal layout to clean vertical stacking.

You gain granular control over the X/Y pixel position of each item.

## Screenshots

**Stacked mode** — battery percentage as a 4th row below the battery icon:

![Stacked mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-stacked.png)

**Inline mode** — percentage shown within the battery icon slot:

![Inline mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-inline.png)

**Off mode** — battery icon only, clean three-icon stack:

![Off mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-off.png)

## How it works

Hooks into the `Taskbar.View.dll` system tray implementation via symbol-based
function hooks. When OmniButton elements appear, the mod forces
`Orientation=Vertical` on the inner StackPanel and positions each icon slot
according to your settings.

## Usage

1. Install the mod via [Windhawk](https://windhawk.net/)
2. The mod applies automatically on startup and after explorer restarts. If icons don't appear within a few seconds, toggle the mod off and back on in Windhawk.
3. Adjust X/Y offsets per mode to pixel-perfect your display

## Settings

Default offsets are tuned for a non-standard Windows 11 taskbar (two rows of taskbar, three rows
of system-tray) in the Windhawk ecosystem. Use the per-mode offsets to align icons for your theme, scaling,
or taskbar layout.

- **Battery percentage** — Off / Inline / Stacked. Changing modes requires restarting explorer.exe.
- **Icon offsets** — each battery mode (Off / Inline / Stacked) has its own X/Y offsets for wifi, volume, battery, and percent. Settings are labeled by mode.

## Windows 11 Taskbar Styler compatibility

This mod does not use the Windows XAML Diagnostics API, so it is compatible
with the Windows 11 Taskbar Styler out of the box — no special settings required.

For basic vertical stacking without battery percentage, paste [style.yaml](https://github.com/sb4ssman/Windhawk-Vertical-OmniButton/blob/main/style.yaml)
into Windows 11 Taskbar Styler → Settings → Advanced.

## Related mods

These mods inspired this one and combine well with it for a fully customized taskbar:

- [Taskbar height and icon size](https://windhawk.net/mods/taskbar-icon-size) — resize the taskbar to give the vertical stack room to breathe
- [Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization) — rich clock formatting options that complement the vertical layout
- [Multirow taskbar for Windows 11](https://windhawk.net/mods/taskbar-multirow) — span taskbar items across multiple rows
- [Taskbar tray icon spacing and grid](https://windhawk.net/mods/taskbar-notification-icon-spacing) — control spacing and grid layout of system tray icons
- [Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler) — full XAML-level taskbar theming; existing style.yaml configs work alongside this mod
