# CLAUDE.md — Virtual Desktop Switcher

## Status: v1.4 — submission prep pass

Single file: [virtual-desktop-switcher.wh.cpp](virtual-desktop-switcher.wh.cpp)
Design doc: [../_claude_notes/vd-switcher-design.md](../_claude_notes/vd-switcher-design.md)

## What it does

Injects a grid of clickable buttons into the taskbar (system tray or edges), one per virtual desktop.
Click switches directly to that desktop. Grid auto-sizes rows from taskbar height.

## Architecture

- Hook: `IconView::IconView` from `SystemTray.dll`/`Taskbar.View.dll`/`ExplorerExtensions.dll` (via `GetSystemTrayModuleHandle`) triggers `ApplyAllSettings` once
- `GetTaskbarXamlRoot` → find `SystemTrayFrameGrid`
- `InjectButtonGrid` → inserts a new `ColumnDefinition`, shifts existing columns, places `Grid`
- Notification thread (STA): `IVirtualDesktopNotificationService` → `Notif_CurrentChanged`
  → `RunFromWindowThread` → `RebuildOrUpdate` on UI thread
- `SwitchToDesktop(int index)` → COM vtable calls, build-specific IIDs (from twinui.pcshell.dll version)
- Desktop names: read from `HKCU\...\VirtualDesktops\Desktops\{GUID}\Name` for tooltips

## Grid layout

Default row-first fill: with 4 desktops in 2 rows -> rows are [1,2] [3,4].
Column-first fill is still available for vertical packing.
Row count: `buttonRows=0` auto-detects from `GetWindowRect(Shell_TrayWnd)`.

## Tray Positions

- `afterClock` — before `ShowDesktopStack` (default)
- `beforeClock` — before `NotificationCenterButton`
- `beforeOmni` — before `ControlCenterButton`
- `beforeIcons` — column 0
- `afterShowDesktop` — after `ShowDesktopStack`

## Start Positions

- `nextToStart` — left of Start; reserves space by pushing `TaskbarFrameRepeater` right
- `overStart` — pure overlay anchored to Start; use `gridVerticalOffset`, padding, and grid settings to nudge
- `rightOfStart` — reserves space to the right of Start by pushing `TaskbarFrameRepeater` right and visually counter-shifting Start back to its original anchor
- Legacy `aboveStart` / `belowStart` values are treated as `overStart` aliases; they are hidden from the settings options.

## Known limitations

- Multi-monitor: only primary taskbar gets buttons.
- `taskbarLeft`/`taskbarRight` were removed: `RootGrid` uses star-sized columns; injecting there collapses them.
- 2026-06-17 pass: Start-overlay is one mode (`overStart`) with existing offset/padding controls; `rightOfStart` reserves space by pushing `TaskbarFrameRepeater.Margin.Left` and counter-shifting the Start button visually. User tested and added screenshots for left/over/right Start cases, including hidden-Start variants.
