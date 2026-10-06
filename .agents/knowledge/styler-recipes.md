# Windows 11 Taskbar Styler — working recipes

Rules the user runs in Windows 11 Taskbar Styler (m417z), each live-confirmed.
Paste each block under the Styler's **controlStyles** setting.

## Taller clock on a side taskbar (confirmed 2026-10-04, 26300.9550)

```yaml
- target: Windows.UI.Xaml.Controls.StackPanel#SystemTrayFrameGrid@OrientationStates > SystemTray.OmniButton#NotificationCenterButton
  styles:
    - Height@VerticalOrientation=96
```

Why it is needed: on a native left/right taskbar Windows pins the clock button
(`NotificationCenterButton`) to `Height=44`, which clips a multi-line Taskbar
Clock Customization clock. On a bottom/top taskbar it has no explicit height.

How it works:
- `@OrientationStates` on the PARENT (`SystemTrayFrameGrid`, the tray's
  StackPanel) makes the style conditional on that parent's visual state group.
  Styler allows the group on the target or any parent, once per target.
- `Height@VerticalOrientation=96` applies only in `VerticalOrientation`; in
  `HorizontalOrientation` Windows' own sizing is left alone.
- Adjust 96 to fit the number of clock lines.

Do not use `DockingStates` for tray rules: it lives on `Grid#RootGrid` under
`Taskbar.TaskbarFrame`, a sibling subtree of `SystemTray.SystemTrayFrame`, so it
is not a parent of any tray element. The tray's `OrientationStates` (and
`LayoutStates` = `NormalLayout` / `VerticalLayout`) on `SystemTrayFrameGrid`
were correct at every edge in the tree dumps
([_research/taskbar-orientation-2026-10.md](../../_research/taskbar-orientation-2026-10.md)).
Per-element `OrientationStates` elsewhere (task-button `IconPanel`s) go stale
and must not be used.

The same pattern works for any tray element whose parent is
`SystemTrayFrameGrid`: `NotifyIconStack`, `NotificationAreaIcons`,
`MainStack`, `NonActivatableStack`, `ControlCenterButton`, `ShowDesktopStack`.
