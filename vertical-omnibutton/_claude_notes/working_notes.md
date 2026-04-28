# Working Notes — vertical-omnibutton

## Current version: 1.2

## Status

All work complete and pushed to PR #3859 on `ramensoftware/windhawk-mods` (branch `sb4ssman-vertical-omnibutton`).

## What works

- Vertical stacking of 3 icons ✓
- Off mode: battery glyph renders normally; % text hidden off-screen ✓
- Inline mode: % in battery slot (3rd row) ✓
- Stacked mode: % as 4th row below battery icon ✓
- All modes apply live (no explorer restart) ✓
- Per-mode X/Y offsets for wifi, volume, battery, and % text ✓
- Uninit restores OmniButton to native horizontal layout ✓
- Settings-changed: clean reset + re-traverse (no stale-element bugs) ✓
- Deferred LayoutUpdated for early-startup timing ✓
- Background retry loop (5× at 2s) in Wh_ModAfterInit ✓

## Architecture summary

- Hooks `winrt::SystemTray::implementation::IconView::IconView` from `Taskbar.View.dll`
- Uses `GetTaskbarXamlRoot` via taskbar.dll symbol hooks (no XAML Diagnostics API)
- `ApplyAllSettings` → finds `ControlCenterButton` → traverses to IsItemsHost StackPanel → `ApplyLayout`
- `OnLayoutUpdated` handles battery slot/inner panel arriving late in the XAML tree
- `CleanupXamlElements` static function used by both Wh_ModUninit and Wh_ModSettingsChanged
- `WalkBatteryTree` (flips inner SP to Vertical) for stacked mode
- `WalkFindInnerSP` (no flip) for off mode — glyph renders normally, only text gets offset
- `WalkFindInlinePercent` for inline mode % positioning

## Calibrated defaults (user's display)

batteryMode: stacked
offWifiX: -2, offBatteryX: 2
inlineWifiX: -2, inlineBatteryX: 2
stackedWifiX: -2, stackedWifiY: 7
stackedBatteryX: 8, stackedBatteryY: -6
stackedPercentX: 2, stackedPercentY: -11

## Pending

- GitHub issue: feature request on `ramensoftware/windhawk` for export/import settings (blurb ready, not yet filed)
- VD switcher mod: design doc at `_claude_notes/vd-switcher-design.md`, to be implemented in a new repo
