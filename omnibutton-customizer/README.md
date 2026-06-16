# OmniButton Customizer

Windhawk mod for rearranging the Windows 11 system tray OmniButton
(wifi, volume/sound, battery, and battery percentage) into a configurable grid.

This grew out of Vertical OmniButton. The original vertical stack is now just
one preset: set `gridColumns` to `1` and use `itemOrder: "wifi volume battery"`.

## What It Does

- Reorders OmniButton items with `itemOrder`
- Lays items out by rows or columns
- Supports 1-column, 2x2, horizontal, and custom grid layouts
- Treats battery percentage as a separate fourth item when Windows exposes it
- Adds per-item X/Y nudges for final pixel alignment
- Keeps the outer OmniButton placement native while sizing the internal items
  host to the configured grid footprint
- Avoids XAML Diagnostics, so it can coexist with Windows 11 Taskbar Styler

## Useful Presets

Standard 2x2:

```text
gridColumns: 2
fillOrder: rowFirst
itemOrder: "wifi volume battery percent"
```

Classic vertical stack, no percent:

```text
gridColumns: 1
itemOrder: "wifi volume battery"
```

Vertical stack with percent:

```text
gridColumns: 1
itemOrder: "wifi volume battery percent"
```

Swap wifi and volume:

```text
itemOrder: "volume wifi battery percent"
```

Column-first 2x2:

```text
gridColumns: 2
fillOrder: columnFirst
itemOrder: "wifi volume battery percent"
```

Original horizontal shape:

```text
gridRows: 1
itemOrder: "wifi volume battery"
```

## Test Checklist

- Enable mod with default 2x2 layout
- Toggle off and verify the native horizontal OmniButton returns
- Test `gridColumns: 1` with `itemOrder: "wifi volume battery"`
- Test `gridColumns: 1` with `itemOrder: "wifi volume battery percent"`
- Test `fillOrder: columnFirst`
- Test `itemOrder: "volume wifi battery percent"`
- Change each nudge setting and verify only that item moves
- Restart Explorer and verify the layout reapplies
- Change settings after restart and verify there are no stale transforms
- Verify the OmniButton stays between the expected tray elements and does not
  overlap the customized clock

## Notes

- Battery percentage must be enabled in Windows for the `percent` item to exist.
- If Windows does not expose battery percentage, the `percent` token is skipped.
- The mod hooks `IconView::IconView` and uses `GetTaskbarXamlRoot` to find the
  live taskbar XAML tree.
- The outer `ControlCenterButton` is not forced to a custom width or position.
  The inner items host reports the grid footprint so Windows can reserve space
  naturally in the tray.
