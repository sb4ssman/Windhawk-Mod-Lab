# OmniButton Customizer

Gives independent layout and appearance control over every native Windows 11
OmniButton item: wifi, volume/sound, battery, and battery percentage. Arrange
them into any grid, hide unneeded items, and tune each item's color, size, font,
opacity, and position. Designed for both standard and multi-row taskbars.

## Battery / percent modes

**Coupled**: the selected battery and percentage elements stay in
their native inner panel and occupy one grid cell as a group. Each still has
independent appearance and nudge controls.

**Independent** (default): battery glyph and percentage text are separate grid
items and can be placed in any positions, including non-adjacent cells.

## Grid settings

- **Grid mode** — Smart automatic, single row/column, or fixed rows/columns/grid.
- **Smart layout** — balanced, vertical packing, or horizontal packing.
- **Slot width / height** — size of each grid cell. Height 0 = taskbar height ÷ rows.
- **Grid rows / columns** — dimensions used by the matching fixed mode.
- Smart automatic keeps a standard-height taskbar in one row and uses multiple
  rows only when the taskbar has enough vertical breathing room.
- **Fill order** — row-first or column-first.
- **Short row or column** — when items don't divide evenly, whether the short
  row/column is first or last, and how it's aligned (start/center/end).

## Item order

`itemOrder` is a space-separated list: `wifi`, `volume`, `battery`, `percent`.
Rearrange tokens to change grid order; omit a token to hide that item. Items
absent from Windows (for example, battery on a desktop) are silently skipped.
In coupled mode, the first `battery` or `percent` token determines the shared
group's position.

## Per-item appearance

Each item has independent color, size, font family, opacity, and X/Y nudge
settings. Group padding controls reserved space around the grid; group X/Y
offsets move the complete OmniButton contents without changing tray ordering.
Colors accept `#RRGGBB`, `#AARRGGBB`, `accent`, `accentLight`, `accentDark`, or
`transparent`. Empty colors/fonts, size 0, and opacity -1 preserve native values.

## Placement and other taskbar mods

`groupPadding*` reserves space inside the native OmniButton and
`groupOffsetX`/`groupOffsetY` visually moves its grid. These controls preserve
the native `ControlCenterButton` in its original system-tray position, so other
mods' semantic anchors such as "before OmniButton" and "before clock" keep
their established meaning. The mod intentionally does not reorder the native
button across tray columns; doing so would require a shared placement lease so
multiple mods cannot claim contradictory anchor order.

## Presets

### Standard 2×2
`gridMode: fixedColumns` · `gridColumns: 2` · `fillOrder: rowFirst` · `batteryPercentMode: independent` · `itemOrder: "wifi, volume, battery, percent"`
(Smart automatic picks this shape on a taller taskbar)

### Single column — 3 icons (no percent)
`gridMode: singleColumn` · `itemOrder: "wifi, volume, battery"`

### Single column — all 4 icons
`gridMode: singleColumn` · `batteryPercentMode: independent` · `itemOrder: "wifi, volume, battery, percent"`

### Percent top, battery bottom (independent mode)
`gridMode: fixedColumns` · `gridColumns: 2` · `batteryPercentMode: independent` · `itemOrder: "wifi, volume, percent, battery"`

### Wide bar (original OmniButton style)
`gridMode: singleRow` · `itemOrder: "wifi, volume, battery"`

## Windows 11 Taskbar Styler compatibility

Does not use XAML Diagnostics. Compatible with Windows 11 Taskbar Styler.
