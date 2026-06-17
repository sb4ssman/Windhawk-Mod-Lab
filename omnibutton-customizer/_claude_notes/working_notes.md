# Working Notes — OmniButton Customizer

## Taskbar Clock Customization Spacer

Scratch source:
`Taskbar-Clock-Customization-Spacer/taskbar-clock-customization-spacer.wh.cpp`

Fork source:
`T:\Github\sb4ssman\m417z-windhawk-mods\mods\taskbar-clock-customization.wh.cpp`

Current PR branch:
`taskbar-clock-elastic-spacer`

Current local commit:
`99c690f Add elastic spacer support to taskbar clock`

Feature summary:

- `%s%` is an elastic spacer token for Windows 11 22H2+ top/bottom clock lines.
- `{spacer}` is supported inside `WebContentWeatherFormat` and is converted
  after weather fetch into the same `%s%` spacer path.
- Spacer rendering is handled in the existing Win11 XAML text-style path by
  generating replacement line elements with star-sized spacer columns.
- `FormatLine` is intentionally left unaware of `%s%`; the token survives
  formatting and is consumed by the XAML rendering layer.

Validation status:

- User confirmed the layout works visually in Windhawk.
- `git diff --check -- mods/taskbar-clock-customization.wh.cpp` passed in the
  fork.
- No repo validation script exists in `m417z-windhawk-mods`.
- Git may still warn about `C:\Users\tmill/.config/git/ignore` permission; this
  is unrelated to the mod patch.

PR direction:

- Initiate from the fork branch.
- Base repository: `m417z/my-windhawk-mods`, branch `main`.
- Compare/head: `sb4ssman/m417z-windhawk-mods`,
  branch `taskbar-clock-elastic-spacer`.

## Current version

v1.0 in development.

This is no longer the narrow Vertical OmniButton mod. It is a generalized
OmniButton grid customizer with arbitrary item ordering, row/column fill, and
per-item nudges.

## What was fixed in the cleanup pass

- Source metadata/logging now agree on `v1.0`.
- Symbol hook arrays are named/commented for Windhawk PR validation.
- README now documents OmniButton Customizer instead of old Vertical OmniButton.
- Folder `CLAUDE.md` now points to `omnibutton-customizer.wh.cpp`.
- Root README now points to `omnibutton-customizer/` instead of the removed
  `vertical-omnibutton/` path.
- Cleanup hardening after user saw OmniButton stuck behind the customized clock:
  clears a broader set of layout properties, invalidates parent layout, and
  rediscover-cleans the live OmniButton even if cached references are stale.
- Layout hardening after the same report: the mod no longer forces the outer
  `ControlCenterButton` width/height/alignment. It sizes only the internal
  items-host StackPanel to the grid footprint, leaving tray placement native.

## Current implementation facts

- `@id`: `omnibutton-customizer`
- Active source: `omnibutton-customizer.wh.cpp`
- Uses `GetTaskbarXamlRoot`, not XAML Diagnostics.
- Hooks `IconView::IconView` with an auto-revoked `Loaded` callback.
- Supports `SystemTray.dll`, old `Taskbar.View.dll`, and `ExplorerExtensions.dll`.
- Uses `LayoutUpdated` to re-apply when battery/percent elements arrive late.
- Uses `RenderTransform` for grid placement and fine nudges.
- Outer OmniButton placement should remain owned by Windows/tray XAML; only
  internal padding/alignment and items-host footprint are customized.

## Known validation status

Local Windhawk PR validator was clean for metadata/symbol-hook issues after the
hook variable cleanup, apart from temp-path warnings when run outside `mods/`.

## Test checklist before submit

- Default 2x2 layout.
- `gridColumns: 1`, `itemOrder: "wifi volume battery"`.
- `gridColumns: 1`, `itemOrder: "wifi volume battery percent"`.
- `gridColumns: 2`, `fillOrder: columnFirst`.
- Swapped order: `itemOrder: "volume wifi battery percent"`.
- `gridRows: 1` horizontal/original-style shape.
- Per-item X/Y nudges.
- Toggle off: native horizontal OmniButton restores.
- Toggle off after several settings changes: OmniButton does not remain behind
  clock/tray content.
- Settings change: no stale transforms or duplicated offsets.
- Explorer restart: layout reapplies.

## Submission posture

PR #3859 is still the old Vertical OmniButton PR. Do not replace it with this
Customizer without making that scope change explicit. The fast low-risk path for
PR #3859 is still the archived fixed `vertical-omnibutton-v1.4.wh.cpp`.

OmniButton Customizer should likely be submitted as a separate new mod after the
test checklist passes.
