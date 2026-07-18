# Tray Privacy Indicator Anchor

A Windhawk mod for Windows 11 that reserves stable tray space for privacy and
status indicators. Location, microphone, camera, and Copilot placeholders stay
visible in the system tray: dim when idle, bright when active.

The goal is to stop taskbar tray sections from shifting when Windows briefly
shows or hides privacy indicators, especially when Windows Web Experience Pack
or Widgets frequently access location.

## Features

- Persistent placeholder icons for location, microphone, camera, and Copilot
- Idle opacity setting so inactive icons can be subtle but still reserve space
- Configurable icon order with `location`, `mic`, `camera`, and `copilot` tokens
- Row-first or column-first grid fill
- Short row/column placement and alignment controls
- Placement before icons, before OmniButton, before clock, after clock, or after Show Desktop
- Per-icon X/Y nudges plus whole-group X/Y offset
- Optional active glow and custom active color
- Optional testing toggle to let Windows' native privacy indicators appear

## Colors

`activeColor` (icons in use) and `slashColor` (the disabled slash overlay)
accept `#RRGGBB` or `#AARRGGBB` hex (the alpha byte is honored), the generics
`accent`, `accentLight`, and `accentDark` for the Windows accent shades, or
`transparent`. Leaving either empty keeps the system foreground color.

## Files

- [privacy-indicator-anchor.wh.cpp](privacy-indicator-anchor.wh.cpp) - Windhawk mod source
- [privacy-trigger-test.html](privacy-trigger-test.html) - local browser test page for triggering privacy states
- [privacy-trigger-server.ps1](privacy-trigger-server.ps1) - helper server for the test page
- [privacy-diag.ps1](privacy-diag.ps1) - diagnostic helper for privacy/device state
- [archive/](archive/) - earlier experiments
- [assets/](assets/) - visual/test assets

## Status

Version `0.9` is still in lab development. Camera and Copilot indicators are
experimental because Windows exposes those states differently across devices and
builds.

## Notes

Camera activation depends on Windows' software camera indicator behavior. Some
devices with hardware camera LEDs may not show a software camera indicator unless
the `NoPhysicalCameraLED` registry behavior applies.

`suppressNativeIndicators` defaults to `1` so the mod hides Windows' own pop-in
privacy indicators and mirrors state into the stable placeholders. Set it to `0`
temporarily when comparing against Windows' native tray glyphs during testing.

Run `privacy-diag.ps1 -Watch` while flipping hardware privacy switches to see
compact microphone and camera state changes without restarting the full report.
