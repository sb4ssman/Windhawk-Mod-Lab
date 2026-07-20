# OmniButton Customizer — live-test checklist

Use the current `omnibutton-customizer.wh.cpp` build. Keep Windhawk logging
open for the battery-mode checks and save the `[Battery]`, `[Layout]`, and
`[Lifecycle]` lines if the result differs from the expected behavior.

## Clean baseline and lifecycle

- Restart Explorer once after installing this candidate. The previous build can
  leave local XAML values in the already-running tree; the new property lease
  must capture a genuinely native baseline.
- Before enabling, verify the native horizontal OmniButton and percentage
  baseline look correct.
- Enable with defaults: independent battery/percentage, Smart automatic grid.
- Change a setting several times; verify transforms/styles do not accumulate.
- Disable the mod; verify the exact native horizontal OmniButton returns and
  the percentage baseline/alignment matches the pre-enable state.
- Re-enable, restart Explorer, and verify the layout reapplies automatically.

## Item contract

- `itemOrder: "wifi volume battery"`: percentage is hidden, not appended.
- `itemOrder: "volume wifi battery percent"`: wifi/volume order swaps.
- Omit each token in turn and verify only that item is hidden.
- On a desktop without a battery, verify unavailable battery tokens are skipped
  without leaving an empty cell.

## Battery modes

- Independent default: battery and percentage occupy separate centered cells.
- Coupled: battery and percentage occupy one grid cell as a native pair.
- Coupled: adjust battery and percentage offsets independently.
- Coupled: omit `percent`; battery remains visible by itself.
- Coupled: omit `battery`; percentage remains visible by itself.
- Independent: `itemOrder: "wifi volume percent battery"`; battery and
  percentage remain visible in separate, non-adjacent cells.
- Toggle coupled ↔ independent repeatedly and confirm no missing glyph, stranded
  percentage, or doubled offset.

## Grid template

- Test Smart automatic with balanced, vertical, and horizontal preferences.
- Test single row and single column.
- Test fixed rows, fixed columns, and fixed grid.
- Test row-first and column-first fill.
- Test first/last short group and start/center/end short-group alignment.
- Test both single-height and multi-row taskbars.
- Force a 2×2 grid on a double-height taskbar; verify all four visuals are
  horizontally and vertically centered before applying any per-item nudge.

## Per-item controls

- Change each item's color independently: hex, alpha hex, `accent`,
  `accentLight`, `accentDark`, `transparent`, and empty/native.
- Change each item's size and font family independently; 0/empty restores native.
- Change each item's opacity independently; -1 restores native and 0 hides the
  visual without removing its grid cell.
- Change every X/Y nudge and verify only the selected item moves.
- Treat nudges as optical corrections only; structural cell centering should
  work at zero.
- Change slot width/height and all four group-padding controls.
- Change group X/Y offsets and verify the complete grid moves while its native
  ordering relative to other tray groups remains unchanged.
