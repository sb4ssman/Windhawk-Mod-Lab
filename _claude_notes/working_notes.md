# Working Notes — vertical-omnibutton

## Current version: 1.49.0

## User goals

1. **Stacked mode**: battery % as a 4th row showing "79%" on ONE row, all 4 items centered horizontally and evenly spaced vertically — functionally working; user can fine-tune visuals with new settings
2. **Inline mode**: battery % shown within the battery slot (3rd row) after explorer restart — functionally working
3. **Off mode**: no % showing — working
4. **Restart button**: reliable, 10-second debounce — working
5. **Pixel-perfect visual alignment** — X/Y offset settings for all icons/text via TranslateTransform (v1.48/v1.49)

## What works reliably

- Vertical stacking of 3 icons after restart ✓
- Vertical clock (time / day / date three rows) ✓
- Explorer restart button with 10s debounce ✓
- Registry key written before restart ✓
- All three battery modes (off/inline/stacked) work after restart ✓
- "79%" is ONE combined TextBlock (not two separate elements) ✓
- Live-switching between modes ✓
- Wifi/volume X/Y offset controls (v1.49) ✓

## Active problems to solve

### Visual pixel-perfection
All modes functionally correct. Per-user fine-tuning via:
- `wifiX`/`wifiY` — TranslateTransform on wifi CP (defaults: 4, 2)
- `volumeX`/`volumeY` — TranslateTransform on volume CP (defaults: 0, 0)
- `batteryGlyphX`/`batteryGlyphY` — stacked glyph row (defaults: 8, -4)
- `batteryPercentX`/`batteryPercentY` — stacked % text row (defaults: 2, -12)
- `batteryInlineX`/`batteryInlineY` — inline battery CP (defaults: 4, 0)

Battery defaults calibrated to user's display. Wifi defaults (4, 2) are estimated — user needs to test v1.49 and confirm or report better values.

## Key facts about the XAML tree

```
SystemTray.OmniButton (Name="ControlCenterButton")
  Grid
    ContentPresenter
      ItemsPresenter
        StackPanel (IsItemsHost=true)     ← g_omniStackPanel, set Orientation=Vertical
          ContentPresenter (wifi)         ← Width=32, Height=28
          ContentPresenter (volume)       ← Width=32, Height=28
          ContentPresenter (battery)      ← g_batteryPresenter
            [when registry=1, contains inner horizontal SP:]
            StackPanel (Horizontal)       ← g_batteryInnerPanel, flipped to Vertical for stacked
              [glyph child]
              [number TextBlock "79"]
              [percent TextBlock "%"]
```

The inner SP structure (direct children: TBs or ContentPresenters wrapping TBs?) is still
not 100% confirmed. Debug logging shows child types but logs haven't been shared this session.

## Settings schema

```yaml
batteryMode: "off" | "inline" | "stacked"   (string, not int)
enableVertical: bool
iconSpacing: 0–20
verticalClock: bool
clockAlignment: "left" | "center" | "right"
clockLineSpacing: 0–20
debugLogging: bool
restartExplorer: bool (toggle, 10s debounce)
```

## Registry

`HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced\TaskbarBatteryPercent`
- Set to 1 for inline/stacked, 0 for off
- Read by explorer ONLY at startup — must restart explorer after changing
- Written BEFORE RestartExplorer() in Wh_ModSettingsChanged
