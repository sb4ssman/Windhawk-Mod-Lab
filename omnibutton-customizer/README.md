# OmniButton Customizer

Rearranges the Windows 11 system tray OmniButton (wifi, volume/sound, battery, battery
percentage) into any grid layout. Designed for multi-row taskbars.

## Battery / percent modes

**Coupled** (default): battery and percent share a slot, rendered side-by-side inside the
native inner panel. Works best when both are in adjacent grid cells.

**Independent**: battery glyph and percent text are each treated as independent grid items
and can be placed at any grid position, including non-adjacent ones. The battery
ContentPresenter spans the full grid footprint and both sub-elements are offset absolutely.

## Grid settings

- **Slot width / height** — size of each grid cell. Height 0 = taskbar height ÷ rows.
- **Grid rows / columns** — 0 = auto: a single column when all items fit the
  taskbar height (double-height taskbars), otherwise more columns — 4 items on
  a single-height taskbar become a 2x2.
- **Fill order** — row-first or column-first.
- **Short row or column** — when items don't divide evenly, whether the short
  row/column is first or last, and how it's aligned (start/center/end).

## Item order

`itemOrder` is a space-separated list: `wifi`, `volume`, `battery`, `percent`.
Rearrange tokens to change which grid cell each item lands in. Items absent from
Windows (e.g. no battery on a desktop) are silently skipped.

## Per-item glyph colors

Set `wifiColor`, `volumeColor`, `batteryColor`, `percentColor` to a hex color
(`#RRGGBB` or `#AARRGGBB`, the alpha byte is honored), the generics `accent`,
`accentLight`, and `accentDark` for the Windows accent shades, or `transparent`.
Leave empty to use the default theme color.

## Presets

### Standard 2×2
`gridColumns: 2` · `fillOrder: rowFirst` · `itemOrder: "wifi volume battery percent"`
(the auto default already picks this shape on a single-height taskbar)

### Single column — 3 icons (no percent)
`gridColumns: 1` · `itemOrder: "wifi volume battery"`

### Single column — all 4 icons
`gridColumns: 1` · `itemOrder: "wifi volume battery percent"`

### Percent top, battery bottom (independent mode)
`batteryPercentMode: independent` · `itemOrder: "wifi volume percent battery"`

### Wide bar (original OmniButton style)
`gridRows: 1` · `itemOrder: "wifi volume battery"`

## Windows 11 Taskbar Styler compatibility

Does not use XAML Diagnostics. Compatible with Windows 11 Taskbar Styler.
