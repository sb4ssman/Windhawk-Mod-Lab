# Privacy Indicator Anchor Design Notes

Last refreshed: 2026-05-02

Goal: stop microphone/location privacy indicators from shifting the centered taskbar when Windows shows/hides them.

## Current Approach

The current mod anchors the real system privacy icon in `MainStack`:

- Find `SystemTray.IconView#SystemTrayIcon` under `SystemTray.Stack#MainStack`.
- Find its `SystemTray.TextIconContent > ... > TextBlock#InnerTextBlock`.
- Watch `TextBlock.TextProperty` to infer active/idle state.
- Watch `UIElement.VisibilityProperty` to force `Visible` when Windows tries to collapse it.
- Set opacity to full when active and idle opacity when inactive.

This preserves the system-owned element, but it still depends on how Windows keeps or clears the glyph while idle.

## User-Requested Direction

Make the privacy indicators live in the ordered notification-icon grid section of the tray, near app tray icons, and keep them permanently visible there.

In Windows 11 taskbar XAML, that area is roughly:

`SystemTray.SystemTrayFrame > Grid#SystemTrayFrameGrid > SystemTray.Stack#NotifyIconStack > ... > SystemTray.StackListView#IconStack`

The current system privacy indicators live separately under:

`SystemTray.Stack#MainStack`

## Recommended Implementation

Do not move the real Windows `IconView` from `MainStack` into `NotifyIconStack`.

Reasons:

- The real privacy `IconView` is owned by Windows' system tray stack/list machinery.
- Moving a live XAML element across parents can conflict with the owner control, virtualization, internal item state, and cleanup.
- If Windows recreates the element, a moved instance may go stale or disappear anyway.

Better design:

1. Keep observing the real `MainStack` privacy indicator as the source of truth.
2. Inject our own persistent XAML element into or adjacent to `NotifyIconStack`.
3. Mirror state into the injected element:
   - idle: dim geolocation/microphone glyph, or a selected fallback glyph
   - active: full opacity and active glyph
   - combined mic+location: combined glyph when the source text reports it
4. Hide or collapse the original `MainStack` privacy indicator to prevent the pop-in/out from shifting layout.
5. On unload, unregister property callbacks, remove the injected element, and restore the original privacy element visibility/opacity.

## Placement Options

Conservative:

- Insert a new `Auto` column in `SystemTrayFrameGrid` immediately before `NotifyIconStack`.
- This is not literally inside the ordered app grid, but it is visually adjacent and uses the stable grid-column insertion pattern already used by VD switcher.

Ambitious:

- Append an injected child into the visual panel under `NotifyIconStack` / `IconStack`.
- This may visually live with app tray icons, but it risks fighting `StackListView` item ownership and ordering logic.

Recommendation for first implementation:

- Add a setting:
  - `location: mainStackAnchor | beforeNotifyIconStack | notifyIconStackExperimental`
- Implement `beforeNotifyIconStack` first.
- Keep `notifyIconStackExperimental` behind a clear experimental label after logging the live tree and verifying cleanup.

## Testing

Use `privacy-trigger-test.html`:

- Start microphone and confirm active glyph/opacity.
- Stop microphone and confirm idle glyph/opacity.
- Start location watch and confirm location path when Windows/browser permissions allow it.
- Toggle the mod off and confirm no stale injected icon remains.
- Test with `taskbar-tray-system-icon-tweaks`, Windows 11 Taskbar Styler, and notification icon spacing/grid mods.

