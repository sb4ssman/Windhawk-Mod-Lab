# Working Notes — vertical-omnibutton

## Current version: 1.51.0

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

None. All modes and offsets are functionally complete and calibrated.

### Calibrated defaults (user's display)
- batteryMode: stacked, wifiOffX:-2, batteryOffX:2
- wifiInlineX:-2, batteryInlineX:2
- wifiStackedX:-2, wifiStackedY:7, batteryGlyphX:8, batteryGlyphY:-6, batteryPercentX:2, batteryPercentY:-11

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
