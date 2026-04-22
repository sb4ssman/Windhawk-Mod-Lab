# Vertical OmniButton — Windhawk Mod

[![Windhawk](https://img.shields.io/badge/Windhawk-Mod-blue)](https://windhawk.net/)
[![Windows 11](https://img.shields.io/badge/Windows-11-blue)](https://www.microsoft.com/windows/windows-11)

Rearranges the Windows 11 system tray OmniButton (wifi, volume/sound, battery) from
horizontal layout to clean vertical stacking.

This mod also includes support for the vertical clock.
You gain granular control over the X/Y pixel position of each item.

## Screenshots

**Stacked mode** — battery percentage as a 4th row below the battery icon:

![Stacked mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-stacked.png)

**Inline mode** — percentage shown within the battery icon slot:

![Inline mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-inline.png)

**Off mode** — battery icon only, clean three-icon stack:

![Off mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-off.png)

## How it works

Uses the Windows XAML Diagnostics API to watch the live XAML visual tree
(the same mechanism used by the Windows 11 Taskbar Styler). When OmniButton
elements appear, the mod forces `Orientation=Vertical` on the inner StackPanel
and positions each icon slot according to your settings.

## Usage

1. Install the mod via [Windhawk](https://windhawk.net/)
2. After enabling, **restart explorer.exe** using the built-in Restart toggle in settings,
   or via Task Manager → Restart explorer.exe
3. Adjust X/Y offsets per mode to pixel-perfect your display

Enable **debug logging** in settings to trace which XAML elements are being checked.

## Settings

- **Enable vertical arrangement** — master toggle for the vertical stack
- **Battery percentage** — Off / Inline / Stacked. Changing modes requires restarting explorer.exe
- **Icon offsets** — each battery mode (Off / Inline / Stacked) has its own X/Y offsets for wifi, volume, and battery
- **Vertical clock** — splits the clock into three rows: time / day / date
- **Debug logging** — log XAML elements as they are added to the visual tree

## Windows 11 Taskbar Styler compatibility

This mod works alongside [Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler).
If you have an existing `style.yaml` or theme for the Taskbar Styler, you can continue using it — the two mods
operate on different parts of the XAML tree and do not conflict.

The style.yaml approach (below) was the original method before this mod existed. It still works as a
no-code alternative if you only need basic vertical stacking without battery percentage or clock support.

## Alternative: style.yaml (no mod required)

You can achieve basic vertical stacking using only the
[Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler).
Go to Settings → Control styles and add the following targets:

**Style 1 - OmniButton Container:**
```
Target: SystemTray.OmniButton#ControlCenterButton
Styles:
  - Margin=0,0,0,0
  - HorizontalContentAlignment=Center
  - Padding=0
  - MinWidth=48
```

**Style 2 - StackPanel Orientation:**
```
Target: SystemTray.OmniButton#ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel
Styles:
  - Orientation=Vertical
  - HorizontalAlignment=Center
  - VerticalAlignment=Center
  - Margin=0
```

**Style 3 - ContentPresenter Sizing:**
```
Target: SystemTray.OmniButton#ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel > ContentPresenter
Styles:
  - Width=32
  - Height=28
  - HorizontalContentAlignment=Right
  - Margin=6,0,10,0
```

**Style 4 - IconView Alignment:**
```
Target: SystemTray.OmniButton#ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel > ContentPresenter > SystemTray.IconView
Styles:
  - HorizontalAlignment=Left
  - VerticalAlignment=Center
  - Margin=0
```

> Note: The style.yaml approach does not support battery percentage rows, vertical clock, or per-icon offset controls.

## Related mods

These mods inspired this one and combine well with it for a fully customized taskbar:

- [Taskbar height and icon size](https://windhawk.net/mods/taskbar-icon-size) — resize the taskbar to give the vertical stack room to breathe
- [Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization) — rich clock formatting options that complement the vertical layout
- [Multirow taskbar for Windows 11](https://windhawk.net/mods/taskbar-multirow) — span taskbar items across multiple rows
- [Taskbar tray icon spacing and grid](https://windhawk.net/mods/taskbar-notification-icon-spacing) — control spacing and grid layout of system tray icons
- [Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler) — full XAML-level taskbar theming; existing style.yaml configs work alongside this mod
