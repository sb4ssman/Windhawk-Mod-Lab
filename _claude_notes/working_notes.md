# Working Notes — vertical-omnibutton

## Current version: 1.43.0

## User goals (NOT yet achieved cleanly)

1. **Stacked mode**: battery % as a 4th row showing "79%" on ONE row (number + % sign together), all 4 items centered horizontally and evenly spaced vertically
2. **Inline mode**: battery % shown within the battery slot (3rd row) after explorer restart — not just on live-switch
3. **Off mode**: no % showing, even without restarting, even if registry was previously set to 1
4. **Restart button**: reliable, 10-second debounce (fixed v1.43)

## What works reliably

- Vertical stacking of 3 icons after restart ✓
- Vertical clock (time / day / date three rows) ✓
- Explorer restart button with 10s debounce ✓
- Registry key written before restart ✓
- Live-switching between modes (without restart) — partially works

## Active problems to solve

### Stacked mode: "79%" still shows as two separate items
`AlignPercentSign` applies `Margin(Left=numW, Top=-lineH)` to the last child of the flipped inner SP.
This SHOULD merge number+% onto one row. Unknown if it's working as intended after v1.43.
Need to test: does stacked mode now show "79%" on one row after a clean restart?

### Inline mode: does it work after restart now?
v1.42 fixed the overwrite bug (late-arriving battery CP getting Width=32/Height=28).
v1.41 fixed ClearValue vs NaN.
v1.43 fixed off-mode ClearValue and stacked flip in settings-changed.
**Needs fresh test with clean restart.**

### Off mode: % appearing when changing other settings
Fixed in v1.43 by removing ClearValue(Width/Height) for batteryMode==0 in settings-changed.
The sizing loop's Width=32/Height=28 on battery CP is now kept.
**Needs verification.**

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
