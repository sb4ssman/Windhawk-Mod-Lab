# CLAUDE.md — Virtual Desktop Switcher

## Status: v1.4 — Start-adjacent placement polish in progress

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

Column-major fill: with 4 desktops in 2 rows → columns are [1,2] [3,4].
Row count: `buttonRows=0` auto-detects from `GetWindowRect(Shell_TrayWnd)`.

## Positions (all within SystemTrayFrameGrid)

- `afterClock` — before `ShowDesktopStack` (default)
- `beforeClock` — before `NotificationCenterButton`
- `beforeOmni` — before `ControlCenterButton`
- `beforeIcons` — column 0
- `afterShowDesktop` — after `ShowDesktopStack`

## Known limitations

- Multi-monitor: only primary taskbar gets buttons.
- `taskbarLeft`/`taskbarRight` were removed: `RootGrid` uses star-sized columns; injecting there collapses them.
- 2026-06-16 pass: `belowStart` is now clamped into the taskbar root, overlay modes honor left/right padding for visible nudging, and `rightOfStart` now tries to reserve taskbar item space via `TaskbarFrameRepeater.Margin.Left`. Needs live testing because Start-adjacent placement depends on Windows' current taskbar XAML tree.
