# Work Log — completed changes

## v1.2 (published to PR #3859 on ramensoftware/windhawk-mods)

Full rewrite from XAML Diagnostics API to GetTaskbarXamlRoot + symbol hooks.

- Changed: switched from XAML Diagnostics to `GetTaskbarXamlRoot` via taskbar.dll symbol hooks — compatible with Windows 11 Taskbar Styler
- Changed: `Wh_ModSettingsChanged` simplified — cleans up current XAML elements via `CleanupXamlElements`, resets globals, calls `ApplyAllSettings` from scratch (no more 400-line duplicate lambda)
- Added: `CleanupXamlElements` static function — called by both uninit and settings-changed
- Added: `WalkFindInnerSP` — finds inner battery StackPanel without flipping orientation, used in Off mode
- Fixed: Off mode battery glyph invisible — was caused by `FlipBatteryLayout` changing inner SP orientation; now `WalkFindInnerSP` leaves orientation native and only pushes the % text off-screen
- Fixed: deferred Off mode in `OnLayoutUpdated` — was missing `ApplyOffset(child, offBatteryX, offBatteryY)` on the outer battery slot
- Fixed: `offPercentX/Y` YAML descriptions removed misleading "Default 0" / "Default 50" text
- Added: deferred wifi/volume sizing in `OnLayoutUpdated` — handles the case where those CPs arrive after initial ApplyLayout
- Removed: `RestoreOmniButtonHeight` (dead code, never called)
- Removed: clock layout feature entirely (`ApplyClockLayout`, `IsClockParent`, clock globals, clock settings) — clock layout delegated to style.yaml + Taskbar Styler
- Synced: battery % language across embedded readme, YAML batteryMode description, and README.md

## v1.51.0 (final calibration)
- Added: `batteryOffX`/`batteryOffY` — battery icon offset in Off mode (was the only icon without its own offset)
- Added: `batteryInlinePercentX`/`batteryInlinePercentY` — offset for the % text element within the inline battery slot, separate from `batteryInlineX/Y` which moves the whole CP
- Added: `WalkFindInlinePercent()` helper — walks battery CP subtree to find inner SP child[1] without flipping orientation
- Added: `g_batteryInlinePercentFE` global to track inline % element for uninit cleanup
- Changed: ApplyLayout mode==0 battery path now calls ApplyOffset with batteryOffX/Y
- Changed: ApplyLayout mode==1 battery path now calls WalkFindInlinePercent after sizing
- Changed: late-add handler mode==0 battery now applies batteryOffX/Y
- Changed: late-add handler mode==1 battery now calls WalkFindInlinePercent
- Changed: settings-changed mode==0 battery path now applies batteryOffX/Y
- Changed: settings-changed mode==1 battery path now applies inline percent offset (re-walks if not yet found)
- Changed: uninit captures and clears g_batteryInlinePercentFE RenderTransform
- Fixed: WalkFindInlinePercent was placed before ApplyOffset in source — moved after so ApplyOffset is declared first (compile error fix)
- Changed: defaults updated to user-calibrated values — batteryMode:"stacked", wifiOffX:-2, batteryOffX:2, wifiInlineX:-2, batteryInlineX:2, wifiStackedY:7, batteryPercentY:-11

## v1.50.0
- Removed: `iconSpacing` setting — superfluous, hardcoded to 0
- Changed: per-mode wifi/volume offsets — `wifiX/Y`/`volumeX/Y` replaced with `wifiOffX/Y`, `wifiInlineX/Y`, `wifiStackedX/Y`, `volumeOffX/Y`, `volumeInlineX/Y`, `volumeStackedX/Y`
- Added: `WifiX()`/`WifiY()`/`VolumeX()`/`VolumeY()` mode-dispatch helpers
- Added: `$if` conditionals on all offset settings — only the settings for the active battery mode are shown in the Windhawk UI
- Changed: batteryMode description reformatted — 3 lines (one per option) plus restart disclaimer
- Added: Related mods section in readme (taskbar height, clock customization, taskbar styler, tray reorder)
- Added: Screenshots section in readme (placeholder with raw GitHub URL format)
- Fixed: debounce comment in Wh_ModSettingsChanged still said "30 s" — corrected to 10 s
- Changed: stacked battery defaults updated to user's calibrated values (batteryGlyphY:-6, wifiStackedX:-2, wifiStackedY:8)

## v1.49.0
- Added: wifiX/wifiY/volumeX/volumeY settings — TranslateTransform offsets for wifi and volume CPs
- Added: `g_wifiPresenter`, `g_volumePresenter` globals to track slots for uninit cleanup
- Changed: ApplyLayout sizing loop captures wifi/volume CPs (slot 0/1) and applies ApplyOffset
- Changed: Late-add handler scans slot index to apply wifi/volume offsets when CPs arrive late
- Changed: Uninit doCleanup captures wifi/vol, nulls globals, clears their RenderTransforms
- Fixed: Latent bug — bip (inner battery SP) children in uninit cleanup were not clearing RenderTransformProperty; TranslateTransform would persist after mod unload if batteryGlyphX/Y or batteryPercentX/Y were nonzero
- Changed: Settings-changed disable cleanup loop now clears RenderTransform on all SP children
- Changed: Battery defaults updated to user-calibrated values (batteryGlyphX:8, batteryGlyphY:-4, batteryPercentX:2, batteryPercentY:-12, batteryInlineX:4, batteryInlineY:0)
- Defaults: wifiX=4, wifiY=2 (measured offset to center wifi glyph and align it down the stack); volumeX=0, volumeY=0

## v1.48.0
- Added: X/Y offset settings for each battery item using TranslateTransform (post-layout, non-destructive):
  - `batteryGlyphX`/`batteryGlyphY` — stacked glyph row (range -20 to 20, default 0)
  - `batteryPercentX`/`batteryPercentY` — stacked % text row
  - `batteryInlineX`/`batteryInlineY` — inline battery CP
- Changed: `ApplyOffset()` helper applies TranslateTransform or ClearValue on RenderTransform
- Changed: `SizeStackedBatteryRows` now calls ApplyOffset instead of Margin
- Changed: RenderTransformProperty cleared in uninit and when switching away from battery mode

## v1.47.0 (superseded by v1.48.0)
- Added height/offset controls via layout manipulation — replaced in v1.48 with TranslateTransform approach

## v1.46.0
- Fixed: stacked mode clipped "%" because Width=32 on battery CP constrained the combined "79%" TextBlock
- Changed: Width=32 moved to glyph Grid inside SizeStackedBatteryRows; battery CP stays Width=NaN
- Added: SizeStackedBatteryRows function to size inner SP children after vertical flip

## v1.45.0
- Fixed: AlignPercentSign was destroying stacked layout — assumed 3 children [glyph, "79", "%"] but actual structure is [Grid, TextBlock "79%"]. Function applied Margin(Top=-22.4) pulling text UP to overlap icon row.
- Removed: AlignPercentSign, FindFirstTextBlock, g_percentTextBlock, g_percentContainer entirely
- Added: SizeStackedBatteryRows — sets glyph Width=32/Height=28/Center; text=Center (no Margin needed)

## v1.44.0
- Verified: inner SP has 2 children — child[0]=Grid (battery glyph), child[1]=TextBlock "79%" (combined, NOT separate "79" and "%")
- This is why AlignPercentSign never worked — it was designed for a structure that doesn't exist

## v1.43.0
- Fixed: off-mode ClearValue(Width/Height) on battery CP in settings-changed caused % to appear when changing unrelated settings (spacing, etc.)
- Fixed: settings-changed for stacked mode now calls FlipBatteryLayout if inner panel not yet flipped — switching OFF→STACKED without restart now actually flips instead of showing inline
- Changed: restart debounce 30s → 10s

## v1.42.0
- Fixed: "Size new FE" handler in OnVisualTreeChange was stamping Width=32/Height=28 on battery CP when it arrived AFTER ApplyLayout found an empty SP. This overwrote every NaN set in ApplyLayout and was the root cause of battery % failing after every restart across v1.36–v1.41.
- Fixed: retry sentinel changed from `!g_percentTextBlock` to `!g_percentContainer` — prevents infinite retries when % is wrapped in ContentPresenter (not a direct TextBlock)

## v1.41.0
- Fixed: inline mode used ClearValue(Width/Height) instead of Width(NaN)/Height(NaN). ClearValue reverts to template default (may be 28px), not unconstrained. NaN forces auto-size as a local override. Stacked mode always used NaN correctly (ClearHeightDescendants); inline didn't.
- Fixed: settings-changed for stacked now re-calls AlignPercentSign after clearing g_percentContainer, so switching away and back to stacked re-applies alignment

## v1.40.0
- Added: AlignPercentSign rewritten — collects all direct FE children (not just TextBlocks), logs their types, uses Margin(Left=numW, Top=-lineH) instead of TranslateTransform. TranslateTransform is post-layout and gets clipped; Margin is layout-level and collapses the extra row.
- Added: FindFirstTextBlock helper for resolving font size from nested TBs
- Added: g_percentContainer global to track the FE that received Margin (for cleanup)
- Added: stacked mode clears Width on battery CP (was only clearing for inline)
- Added: retry condition changed to !g_percentContainer

## v1.39.0
- Fixed: AlignPercentSign was defined but never called — now called after FlipBatteryLayout and in deferred flip path
- Fixed: inline mode in ApplyLayout was only clearing Width, not Height (settings-changed path cleared both — mismatch)

## v1.38.0
- Fixed: ApplyBatteryPercent now called BEFORE RestartExplorer in Wh_ModSettingsChanged. New explorer reads registry at startup; setting it after the kill was too late.
- Added: FreeOmniButtonHeight — sets g_omniButton.Height(NaN) + InvalidateMeasure on 2 parent levels

## v1.37.0
- Fixed: restart toggle was edge-detect initialized from current setting, so after a restart with toggle=true it never fired again. Replaced with 30s tick-based debounce (GetTickCount).
- Fixed: deferred flip now walks up to g_omniStackPanel to find battery CP even if g_batteryPresenter was never set

## v1.36.0
- Added: deferred flip in OnVisualTreeChange for when inner SP arrives after ApplyLayout
- Added: ClearHeightDescendants runs unconditionally before FlipBatteryLayout

## v1.35.0
- Switched ClearHeightDescendants to walk DOWN (GetChildrenCount/GetChild) instead of UP (GetParent returns null during Add callbacks). Sets Height=NaN as local value to override template.

## v1.34.0
- Attempted: ClearHeightChain walking UP via GetParent. FAILED: GetParent returns null during Add callbacks because parents aren't wired yet.

## v1.33.0
- Attempted: ClearValue(HeightProperty) on battery CP. FAILED: reverts to template value (28px).

## v1.32.0
- Attempted: fixed battery CP height 90px. FAILED: overflow — wifi+vol+battery = 146px, clipped.

## v1.30–v1.31
- Attempted: AlignPercentSign using tb.Text() == L"%" to find % TextBlock. FAILED: data-bound TextBlocks return empty string on first tree build. Switched to positional (last two TBs).

## Architecture decisions (permanent)

- XAML Diagnostics API (InitializeXamlDiagnosticsEx) chosen over LoadLibraryExW, LoadPackagedLibrary, and vtable hooks — all of which fail for different reasons (see Claude_notes.md)
- batteryMode stored as int internally (0/1/2), parsed from string settings ("off"/"inline"/"stacked")
- Registry NOT restored on Wh_ModUninit — new explorer would read 0 before Wh_ModInit re-applies it
- FreeLibrary(self) in OmniButtonTAP::SetSite balances the LoadLibrary that IXDE does on the mod DLL
