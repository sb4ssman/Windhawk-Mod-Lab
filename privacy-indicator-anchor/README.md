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
- Per-icon X/Y nudges plus whole-bar X/Y offset
- Optional active glow and custom active color

## Settings Compatibility

The mod keeps its existing user-facing icon names, such as `iconSpacing` and
`barOffsetX/Y`, because they describe the privacy indicator UI clearly. The code
also accepts the shared taskbar-control aliases `buttonSpacing`,
`groupPaddingLeft/Right`, `groupOffsetX/Y`, and `fillOrder` for consistency with
the other lab mods.

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
