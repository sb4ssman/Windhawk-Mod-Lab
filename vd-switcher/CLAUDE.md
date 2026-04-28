# CLAUDE.md — VD Switcher

## Status: v0.1 — first draft, untested

Single file: [vd-switcher.wh.cpp](vd-switcher.wh.cpp)
Design doc: [../_claude_notes/vd-switcher-design.md](../_claude_notes/vd-switcher-design.md)

## What it does

Injects a grid of clickable buttons into the taskbar (system tray or edges), one per virtual desktop.
Click switches directly to that desktop. Grid auto-sizes rows from taskbar height.

## Architecture

- Hook: `IconView::IconView` from `Taskbar.View.dll` triggers `ApplyAllSettings` once
- `GetTaskbarXamlRoot` → find `SystemTrayFrameGrid` (or `RootGrid` for taskbar-edge positions)
- `InjectButtonGrid` → creates `Grid` control, inserts at chosen position index
- Notification thread (STA): `IVirtualDesktopNotificationService` → `Notif_CurrentChanged`
  → `RunFromWindowThread` → `RebuildOrUpdate` on UI thread
- `SwitchToDesktop(int index)` → COM vtable calls, build-specific IIDs (from twinui.pcshell.dll version)

## Grid layout

Column-major fill: with 4 desktops in 2 rows → columns are [1,2] [3,4].
Row count: `buttonRows=0` auto-detects from `GetWindowRect(Shell_TrayWnd)`.

## Positions

System tray (parent = `SystemTrayFrameGrid`):
- `afterClock` — before `ShowDesktopStack` (default)
- `beforeClock` — before `NotificationCenterButton`
- `beforeOmni` — before `ControlCenterButton`
- `beforeIcons` — index 0
- `afterShowDesktop` — append

Taskbar-wide (parent = `RootGrid`):
- `taskbarLeft` — index 0
- `taskbarRight` — before `SystemTrayFrameGrid`

## Known issues / v0.1 limitations

- Buttons don't auto-update when desktops are added/removed without switching.
  Fix: implement `DesktopCreated`/`DesktopDestroyed` vtable slots in notification object,
  or add a poll thread.
- Multi-monitor: only primary taskbar gets buttons.
- `taskbarLeft`/`taskbarRight` placement in `RootGrid` (a Grid with defined columns) may
  have sizing quirks — the bar lands in column 0 by default.

## Testing sequence

1. Load mod in Windhawk — buttons should appear in tray after clock
2. Create/switch desktops — verify highlighting updates
3. Change position setting — buttons move, no leftover elements
4. Toggle mod off — buttons removed cleanly
5. Test all 7 position values
