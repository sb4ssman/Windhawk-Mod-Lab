# Work Log — completed changes

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
