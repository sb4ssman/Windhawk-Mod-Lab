# Bug History and Fixes

## Confirmed root causes (v1.36–v1.42)

### The overwrite bug (fixed v1.42)
`OnVisualTreeChange` has a handler at the bottom that fires when any new FE is added
directly to `g_omniStackPanel`. When the IsItemsHost SP arrives with 0 children,
`ApplyLayout` runs and finds nothing. The battery CP arrives LATER as a new Add event.
This handler stamped `Width=32, Height=28` on it UNCONDITIONALLY — overwriting any NaN
set in ApplyLayout. This is why every version from v1.36–v1.41 produced identical results:
we were fixing values above the overwrite while the overwrite kept firing.
**Fixed**: handler now detects battery slot and applies NaN for inline/stacked modes.

### The spacing-triggers-% bug (fixed v1.43)
`Wh_ModSettingsChanged` has a sizing loop that sets `Width=32, Height=28` on all SP children.
Then for `batteryMode==0`, the battery-specific block called `bp.ClearValue(Width/Height)`.
This REMOVED the 32×28 constraint from the sizing loop, letting the native template render
at full size. If registry=1 (set from a prior inline/stacked session), the template includes
% elements which then became visible. Triggered by any settings change including spacing.
**Fixed**: removed `ClearValue` for mode==0; sizing loop's 32×28 is kept as-is.

### The stacked-shows-inline bug (fixed v1.43)
`Wh_ModSettingsChanged` for `batteryMode==2` never called `FlipBatteryLayout`. If you
changed OFF→STACKED without restarting explorer, the inner SP stayed horizontal.
AlignPercentSign then had nothing to work with, and the battery CP just showed
`[icon][79][%]` horizontally — same as inline, but with possible vertical misalignment.
**Fixed**: settings-changed for stacked now calls `FlipBatteryLayout(bp)` if inner panel
not yet flipped, then re-runs AlignPercentSign.

### The ClearValue-vs-NaN bug (fixed v1.41)
For inline mode in `ApplyLayout`, we used `ClearValue(WidthProperty)` / `ClearValue(HeightProperty)`.
`ClearValue` reverts to the **template value**, not to unconstrained. If the OmniButton template
sets fixed Width/Height on its ContentPresenter children, this reverts to those values (likely 28px).
Must use `Width(NaN)` / `Height(NaN)` to force auto-size as a local override.
The code comment on `ClearHeightDescendants` literally says this — stacked mode always did it
right, inline mode didn't until v1.41.

## Behavioral facts (confirmed across many sessions)

1. Vertical 3-icon stacking works perfectly after restart.
2. Live-switching (settings change without restart) works because the tree is already rendered.
3. After any explorer restart, battery % modes fail — until v1.42 fixed the overwrite bug.
4. `TranslateTransform` (old AlignPercentSign) had zero visible effect.
   Replaced with `Margin(Left=numW, Top=-lineH)` in v1.40.
5. `AlignPercentSign` checks `!g_percentContainer` (not `!g_percentTextBlock`) as the retry
   sentinel — otherwise retries forever if % is wrapped in a ContentPresenter.

## Architecture notes

### Two paths for battery CP sizing
1. **ApplyLayout path**: fires when IsItemsHost SP is added AND has children. Sets NaN for
   inline/stacked on battery CP. Reliable if children already exist.
2. **Late Add path** (OnVisualTreeChange bottom handler): fires when battery CP arrives AFTER
   ApplyLayout found empty SP. Must also apply NaN for inline/stacked. Fixed in v1.42.

### Inner SP flip
- Done by `WalkBatteryTree` → `FlipBatteryLayout` in `ApplyLayout` (if inner SP already there)
- Done by deferred flip in `OnVisualTreeChange` (if inner SP arrives later)
- Done by `FlipBatteryLayout` in `Wh_ModSettingsChanged` stacked branch (v1.43, if not yet flipped)

### Registry timing
`TaskbarBatteryPercent` is read at explorer STARTUP. Must call `ApplyBatteryPercent` BEFORE
`RestartExplorer` in `Wh_ModSettingsChanged`. This has been correct since v1.38.

### Restart debounce
Changed from 30s to 10s in v1.43 per user request.
