# Windhawk Mod: Vertical System Tray OmniButton Icons

[![Windhawk](https://img.shields.io/badge/Windhawk-Mod-blue)](https://windhawk.net/)
[![Windows 11](https://img.shields.io/badge/Windows-11-blue)](https://www.microsoft.com/windows/windows-11)
[![Status](https://img.shields.io/badge/Status-In%20Development-yellow)](https://github.com/tmiller711/Windhawk-Vertical-Omnibutton)

Stack Windows 11 system tray icons (wifi, volume, battery) vertically using the **[Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler)** mod.

## Quick Setup

1. Install **[Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler)** from Windhawk
2. Go to Settings → Control styles
3. Copy-paste the style.txt directly, or for each component:

Choosing Windows Vista Theme works best visually. Adjust the padding and margins to fit your taste.

**Style 1 - OmniButton Container:**
```
Target: SystemTray.OmniButton#ControlCenterButton
Styles:
  - Margin=0,0,0,0
  - HorizontalContentAlignment=Center
  - Padding=0
  - MinWidth=48
```

**Style 2 - StackPanel Orientation:**
```
Target: SystemTray.OmniButton#ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel
Styles:
  - Orientation=Vertical
  - HorizontalAlignment=Center
  - VerticalAlignment=Center
  - Margin=0
```

**Style 3 - ContentPresenter Sizing:**
```
Target: SystemTray.OmniButton#ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel > ContentPresenter
Styles:
  - Width=32
  - Height=28
  - HorizontalContentAlignment=Right
  - Margin=6,0,10,0
```

**Style 4 - IconView Alignment:**
```
Target: SystemTray.OmniButton#ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel > ContentPresenter > SystemTray.IconView
Styles:
  - HorizontalAlignment=Left
  - VerticalAlignment=Center
  - Margin=0
```

## Notes

- Icons stack vertically but may not be perfectly centered
- No custom C++ mod needed - uses existing Taskbar Styler
- See `archive/` folder for failed C++ implementation attempts

## Credits

Solution uses **[Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler)** by m417z via [Windhawk](https://windhawk.net/)

---

**Configuration documented by**: sb4ssman
