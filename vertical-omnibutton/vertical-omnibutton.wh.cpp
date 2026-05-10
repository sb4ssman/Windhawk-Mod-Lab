// ==WindhawkMod==
// @id              vertical-omnibutton
// @name            Vertical OmniButton
// @description     Stacks Windows 11 wifi/volume/battery OmniButton vertically
// @version         1.4
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Vertical OmniButton

Rearranges the Windows 11 system tray OmniButton (wifi, volume/sound, battery) from
horizontal layout to clean vertical stacking.

You gain granular control over the X-Y pixel location of each item in the button.


## Screenshots (clock modded separately)

**Stacked mode** — battery percentage as a 4th row below the battery icon:

![Stacked mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-stacked.png)

**Inline mode** — percentage shown within the battery icon slot:

![Inline mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-inline.png)

**Off mode** — battery icon only, clean three-icon stack:

![Off mode](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-off.png)

## How it works

Hooks into the Windows 11 system tray implementation via symbol-based function hooks
(`SystemTray.dll` on newer builds, `Taskbar.View.dll` on older ones). When OmniButton
elements appear, the mod forces `Orientation=Vertical` on the inner StackPanel and
positions each icon slot according to your settings.

## Usage

The mod applies automatically on startup and after explorer restarts. If icons don't appear within a few seconds of enabling the mod, toggle it off and back on in Windhawk.

## Settings

Default offsets are tuned for a non-standard Windows 11 taskbar (two rows of taskbar, three rows 
of system-tray) in the Windhawk ecosystem. Use the per-mode offsets to align icons for your theme, scaling,
or taskbar layout.

- **Battery percentage** — Off / Inline / Stacked. All modes apply live. The mod expects battery percentage to be enabled in Windows Settings (System → Power & battery → Show battery percentage). In Off mode it is drawn off-screen via the offset settings. If battery percentage is disabled in Windows, all three modes look the same (battery icon only).
- **Button horizontal padding** — adjusts the overall OmniButton width while keeping the 32px icon column and per-icon X/Y offsets intact. Lower it to reduce the gap between the OmniButton, neighboring tray icons, and the clock.
- **Icon offsets** — each battery mode (Off / Inline / Stacked) has its own
  X/Y offsets for wifi, volume, battery, and percent. Settings are labeled by mode.

## Windows 11 Taskbar Styler compatibility

This mod does not use the Windows XAML Diagnostics API, so it is compatible
with the Windows 11 Taskbar Styler out of the box — no special settings required.

For basic vertical stacking without battery percentage, paste [style.yaml](https://github.com/sb4ssman/Windhawk-Vertical-OmniButton/blob/main/style.yaml)
into Windows 11 Taskbar Styler → Settings → Textual mode.

## Related mods

These mods inspired this one and combine well with it for a fully customized taskbar:

- [Taskbar height and icon size](https://windhawk.net/mods/taskbar-icon-size) — resize the taskbar to give the vertical stack room to breathe
- [Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization) — rich clock formatting options that complement the vertical layout
- [Multirow taskbar for Windows 11](https://windhawk.net/mods/taskbar-multirow) — span taskbar items across multiple rows
- [Taskbar tray icon spacing and grid](https://windhawk.net/mods/taskbar-notification-icon-spacing) — control spacing and grid layout of system tray icons
- [Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler) — full XAML-level taskbar theming; existing style.yaml configs work alongside this mod

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- batteryMode: "stacked"
  $name: Battery percentage
  $description: "Off: battery icon only — percentage is drawn off-screen (see Off mode percent settings below).\nInline: percentage shown in the battery icon slot (3rd row).\nStacked: percentage as a separate 4th row below the battery icon.\n\nAll modes apply live — no explorer restart needed. The mod expects battery percentage to be enabled in Windows Settings (System → Power & battery → Show battery percentage). In Off mode it is hidden via the offset settings. If battery percentage is disabled in Windows, all three modes look the same (battery icon only).\n\nDefaults are tuned for a non-standard Windows 11 taskbar (two rows of taskbar, three rows of system-tray) in the Windhawk ecosystem. Use the per-mode offsets below to align icons for your theme, scaling, or taskbar layout. Suggestions welcome — open an issue or PR on GitHub."
  $options:
    - "off": "Off — battery icon only"
    - "inline": "Inline — percentage in battery slot (3rd row)"
    - "stacked": "Stacked — percentage as 4th row below battery"
    - "grid": "Grid — wifi/volume top row, battery/% bottom row"
- buttonHorizontalPadding: 8
  $name: Button horizontal padding
  $description: "Horizontal padding on each side of the 32px icon column. Lower values make the OmniButton narrower so it sits closer to neighboring tray icons and the clock. Range: 0-24. Default: 8 (about 48px total width)."
- offWifiX: -2
  $name: "Off mode: Wifi X"
  $description: "Wifi horizontal offset when battery % is Off. Negative = left, positive = right."
- offWifiY: 0
  $name: "Off mode: Wifi Y"
  $description: "Wifi vertical offset when battery % is Off. Negative = up, positive = down."
- offVolumeX: 0
  $name: "Off mode: Volume X"
  $description: "Volume horizontal offset when battery % is Off. Negative = left, positive = right."
- offVolumeY: 0
  $name: "Off mode: Volume Y"
  $description: "Volume vertical offset when battery % is Off. Negative = up, positive = down."
- offBatteryX: 2
  $name: "Off mode: Battery X"
  $description: "Battery icon horizontal offset when battery % is Off. Negative = left, positive = right."
- offBatteryY: 0
  $name: "Off mode: Battery Y"
  $description: "Battery icon vertical offset when battery % is Off. Negative = up, positive = down."
- offPercentX: 0
  $name: "Off mode: Battery percent X"
  $description: "Horizontal offset applied to the battery percentage text in Off mode to push it out of view. Adjust if the text bleeds into view on your theme."
- offPercentY: 50
  $name: "Off mode: Battery percent Y"
  $description: "Vertical offset applied to the battery percentage text in Off mode to push it out of view. Increase if the text bleeds into view on your theme; decrease if the slot clips it cleanly already."
- inlineWifiX: -2
  $name: "Inline mode: Wifi X"
  $description: "Wifi horizontal offset in Inline mode. Negative = left, positive = right."
- inlineWifiY: 0
  $name: "Inline mode: Wifi Y"
  $description: "Wifi vertical offset in Inline mode. Negative = up, positive = down."
- inlineVolumeX: 0
  $name: "Inline mode: Volume X"
  $description: "Volume horizontal offset in Inline mode. Negative = left, positive = right."
- inlineVolumeY: 0
  $name: "Inline mode: Volume Y"
  $description: "Volume vertical offset in Inline mode. Negative = up, positive = down."
- inlineBatteryX: 2
  $name: "Inline mode: Battery X"
  $description: "Battery slot horizontal offset in Inline mode. Negative = left, positive = right. Default: 2."
- inlineBatteryY: 0
  $name: "Inline mode: Battery Y"
  $description: "Battery slot vertical offset in Inline mode. Negative = up, positive = down."
- inlinePercentX: 2
  $name: "Inline mode: Battery percent X"
  $description: "Percentage text horizontal offset within the inline battery slot. Negative = left, positive = right."
- inlinePercentY: -1
  $name: "Inline mode: Battery percent Y"
  $description: "Percentage text vertical offset within the inline battery slot. Negative = up, positive = down."
- stackedWifiX: -2
  $name: "Stacked mode: Wifi X"
  $description: "Wifi horizontal offset in Stacked mode. Negative = left, positive = right."
- stackedWifiY: 7
  $name: "Stacked mode: Wifi Y"
  $description: "Wifi vertical offset in Stacked mode. Negative = up, positive = down."
- stackedVolumeX: 0
  $name: "Stacked mode: Volume X"
  $description: "Volume horizontal offset in Stacked mode. Negative = left, positive = right."
- stackedVolumeY: 0
  $name: "Stacked mode: Volume Y"
  $description: "Volume vertical offset in Stacked mode. Negative = up, positive = down."
- stackedBatteryX: 8
  $name: "Stacked mode: Battery X"
  $description: "Battery icon row horizontal offset in Stacked mode. Negative = left, positive = right. Default: 8."
- stackedBatteryY: -6
  $name: "Stacked mode: Battery Y"
  $description: "Battery icon row vertical offset in Stacked mode. Negative = up, positive = down. Default: -6."
- stackedPercentX: 2
  $name: "Stacked mode: Battery percent X"
  $description: "Percentage row horizontal offset in Stacked mode. Negative = left, positive = right. Default: 2."
- stackedPercentY: -11
  $name: "Stacked mode: Battery percent Y"
  $description: "Percentage row vertical offset in Stacked mode. Negative = up, positive = down. Default: -11."
- gridSlotWidth: 34
  $name: "Grid mode: Column width (px)"
  $description: >-
    Width of each column in the 2x2 grid. Controls how far Volume and Battery %%
    move into the right column. Default 34 = 32px icon + 2px gap.
- gridWifiX: 0
  $name: "Grid mode: Wifi X"
  $description: "Fine-tune wifi horizontal position in grid mode."
- gridWifiY: 0
  $name: "Grid mode: Wifi Y"
  $description: "Fine-tune wifi vertical position in grid mode."
- gridVolumeX: 0
  $name: "Grid mode: Volume X"
  $description: "Fine-tune volume horizontal offset in grid mode (added on top of the column offset)."
- gridVolumeY: 0
  $name: "Grid mode: Volume Y"
  $description: "Fine-tune volume vertical offset in grid mode (added on top of the -28px row offset)."
- gridBatteryX: 0
  $name: "Grid mode: Battery X"
  $description: "Fine-tune battery icon horizontal position in grid mode."
- gridBatteryY: 0
  $name: "Grid mode: Battery Y"
  $description: "Fine-tune battery icon vertical offset in grid mode (added on top of the -28px row offset)."
- gridPercentX: 0
  $name: "Grid mode: Battery %% X"
  $description: "Fine-tune battery percentage horizontal position in grid mode."
- gridPercentY: 0
  $name: "Grid mode: Battery %% Y"
  $description: "Fine-tune battery percentage vertical position in grid mode."
*/
// ==/WindhawkModSettings==

#include <atomic>
#include <functional>
#include <limits>
#include <list>
#include <winrt/base.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime

#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;
using winrt::Windows::Foundation::IInspectable;

// ── Settings ───────────────────────────────────────────────────────────────

struct {
    int  batteryMode;          // 0=off, 1=inline (3rd row), 2=stacked (4th row), 3=grid 2x2
    int  buttonHorizontalPadding;
    int  offWifiX,    offWifiY;
    int  inlineWifiX, inlineWifiY;
    int  stackedWifiX,stackedWifiY;
    int  offVolumeX,    offVolumeY;
    int  inlineVolumeX, inlineVolumeY;
    int  stackedVolumeX,stackedVolumeY;
    int  offBatteryX,     offBatteryY;
    int  offPercentX,     offPercentY;
    int  stackedBatteryX,   stackedBatteryY;
    int  stackedPercentX, stackedPercentY;
    int  inlineBatteryX,  inlineBatteryY;
    int  inlinePercentX, inlinePercentY;
    int  gridSlotWidth;
    int  gridWifiX,    gridWifiY;
    int  gridVolumeX,  gridVolumeY;
    int  gridBatteryX, gridBatteryY;
    int  gridPercentX, gridPercentY;
} g_settings;

std::atomic<bool> g_unloading = false;

void LoadSettings() {
    {
        auto* bm = Wh_GetStringSetting(L"batteryMode");
        if (bm) {
            if (wcscmp(bm, L"inline") == 0)       g_settings.batteryMode = 1;
            else if (wcscmp(bm, L"stacked") == 0) g_settings.batteryMode = 2;
            else if (wcscmp(bm, L"grid") == 0)    g_settings.batteryMode = 3;
            else                                   g_settings.batteryMode = 0;
            Wh_FreeStringSetting(bm);
        } else {
            g_settings.batteryMode = 0;
        }
    }
    auto clampOffset = [](int v) { return v < -20 ? -20 : v > 20 ? 20 : v; };
    auto clampHide   = [](int v) { return v < -100 ? -100 : v > 100 ? 100 : v; };
    auto clampPadding = [](int v) { return v < 0 ? 0 : v > 24 ? 24 : v; };
    g_settings.buttonHorizontalPadding = clampPadding(Wh_GetIntSetting(L"buttonHorizontalPadding"));
    g_settings.offWifiX      = clampOffset(Wh_GetIntSetting(L"offWifiX"));
    g_settings.offWifiY      = clampOffset(Wh_GetIntSetting(L"offWifiY"));
    g_settings.inlineWifiX   = clampOffset(Wh_GetIntSetting(L"inlineWifiX"));
    g_settings.inlineWifiY   = clampOffset(Wh_GetIntSetting(L"inlineWifiY"));
    g_settings.stackedWifiX  = clampOffset(Wh_GetIntSetting(L"stackedWifiX"));
    g_settings.stackedWifiY  = clampOffset(Wh_GetIntSetting(L"stackedWifiY"));
    g_settings.offVolumeX    = clampOffset(Wh_GetIntSetting(L"offVolumeX"));
    g_settings.offVolumeY    = clampOffset(Wh_GetIntSetting(L"offVolumeY"));
    g_settings.inlineVolumeX = clampOffset(Wh_GetIntSetting(L"inlineVolumeX"));
    g_settings.inlineVolumeY = clampOffset(Wh_GetIntSetting(L"inlineVolumeY"));
    g_settings.stackedVolumeX= clampOffset(Wh_GetIntSetting(L"stackedVolumeX"));
    g_settings.stackedVolumeY= clampOffset(Wh_GetIntSetting(L"stackedVolumeY"));
    g_settings.offBatteryX     = clampOffset(Wh_GetIntSetting(L"offBatteryX"));
    g_settings.offBatteryY     = clampOffset(Wh_GetIntSetting(L"offBatteryY"));
    g_settings.offPercentX     = clampHide(Wh_GetIntSetting(L"offPercentX"));
    g_settings.offPercentY     = clampHide(Wh_GetIntSetting(L"offPercentY"));
    g_settings.stackedBatteryX   = clampOffset(Wh_GetIntSetting(L"stackedBatteryX"));
    g_settings.stackedBatteryY   = clampOffset(Wh_GetIntSetting(L"stackedBatteryY"));
    g_settings.stackedPercentX = clampOffset(Wh_GetIntSetting(L"stackedPercentX"));
    g_settings.stackedPercentY = clampOffset(Wh_GetIntSetting(L"stackedPercentY"));
    g_settings.inlineBatteryX  = clampOffset(Wh_GetIntSetting(L"inlineBatteryX"));
    g_settings.inlineBatteryY  = clampOffset(Wh_GetIntSetting(L"inlineBatteryY"));
    g_settings.inlinePercentX = clampOffset(Wh_GetIntSetting(L"inlinePercentX"));
    g_settings.inlinePercentY = clampOffset(Wh_GetIntSetting(L"inlinePercentY"));
    auto clampSlot = [](int v) { return v < 16 ? 16 : v > 60 ? 60 : v; };
    g_settings.gridSlotWidth  = clampSlot(Wh_GetIntSetting(L"gridSlotWidth"));
    g_settings.gridWifiX      = clampOffset(Wh_GetIntSetting(L"gridWifiX"));
    g_settings.gridWifiY      = clampOffset(Wh_GetIntSetting(L"gridWifiY"));
    g_settings.gridVolumeX    = clampOffset(Wh_GetIntSetting(L"gridVolumeX"));
    g_settings.gridVolumeY    = clampOffset(Wh_GetIntSetting(L"gridVolumeY"));
    g_settings.gridBatteryX   = clampOffset(Wh_GetIntSetting(L"gridBatteryX"));
    g_settings.gridBatteryY   = clampOffset(Wh_GetIntSetting(L"gridBatteryY"));
    g_settings.gridPercentX   = clampOffset(Wh_GetIntSetting(L"gridPercentX"));
    g_settings.gridPercentY   = clampOffset(Wh_GetIntSetting(L"gridPercentY"));
}

static int WifiX()   {
    switch (g_settings.batteryMode) {
        case 1:  return g_settings.inlineWifiX;
        case 2:  return g_settings.stackedWifiX;
        case 3:  return g_settings.gridWifiX;
        default: return g_settings.offWifiX;
    }
}
static int WifiY()   {
    switch (g_settings.batteryMode) {
        case 1:  return g_settings.inlineWifiY;
        case 2:  return g_settings.stackedWifiY;
        case 3:  return g_settings.gridWifiY;
        default: return g_settings.offWifiY;
    }
}
static int VolumeX() {
    switch (g_settings.batteryMode) {
        case 1:  return g_settings.inlineVolumeX;
        case 2:  return g_settings.stackedVolumeX;
        case 3:  return g_settings.gridSlotWidth + g_settings.gridVolumeX;
        default: return g_settings.offVolumeX;
    }
}
static int VolumeY() {
    switch (g_settings.batteryMode) {
        case 1:  return g_settings.inlineVolumeY;
        case 2:  return g_settings.stackedVolumeY;
        case 3:  return -28 + g_settings.gridVolumeY;
        default: return g_settings.offVolumeY;
    }
}

// ── Cached element references ─────────────────────────────────────────────

static StackPanel       g_omniStackPanel{ nullptr };
static FrameworkElement g_omniButton{ nullptr };
static FrameworkElement g_wifiPresenter{ nullptr };
static FrameworkElement g_volumePresenter{ nullptr };
static FrameworkElement g_batteryPresenter{ nullptr };
static StackPanel       g_batteryInnerPanel{ nullptr };
static FrameworkElement g_batteryInlinePercentFE{ nullptr };

static StackPanel       g_layoutUpdatedSP{ nullptr };
static winrt::event_token g_layoutUpdatedToken{};

static HWND   g_taskbarWnd       = nullptr;

static std::list<FrameworkElement::Loaded_revoker> g_autoRevokerList;

// ── Battery XAML helpers ──────────────────────────────────────────────────

static bool HasBatteryDescendant(DependencyObject const& node, int depth = 0) {
    if (depth > 3) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        std::wstring cls(winrt::get_class_name(child).c_str());
        if (cls.find(L"Battery") != std::wstring::npos) return true;
        if (HasBatteryDescendant(child, depth + 1)) return true;
    }
    return false;
}

static bool WalkBatteryTree(DependencyObject const& node, int depth) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost()) {
            bool wasVertical = sp.Orientation() == Orientation::Vertical;
            sp.Orientation(Orientation::Vertical);
            g_batteryInnerPanel = sp;
            Wh_Log(L"[Battery4] Found inner SP at depth %d (%s → Vertical)",
                depth, wasVertical ? L"was Vertical" : L"was Horizontal");
            return true;
        }
        if (WalkBatteryTree(child, depth + 1)) return true;
    }
    return false;
}

static void FlipBatteryLayout(FrameworkElement const& batteryCP) {
    if (!WalkBatteryTree(batteryCP, 0))
        Wh_Log(L"[Battery4] No inner StackPanel found (%% may not be in tree yet)");
}

// Like WalkBatteryTree but does NOT flip orientation — used in Off mode so the
// glyph continues to render exactly as Windows intends; we only push the text.
static bool WalkFindInnerSP(DependencyObject const& node, int depth = 0) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost()) {
            g_batteryInnerPanel = sp;
            return true;
        }
        if (WalkFindInnerSP(child, depth + 1)) return true;
    }
    return false;
}

static void ApplyOffset(FrameworkElement const& fe, int x, int y) {
    if (x != 0 || y != 0) {
        TranslateTransform tt;
        tt.X(static_cast<double>(x));
        tt.Y(static_cast<double>(y));
        fe.RenderTransform(tt);
    } else {
        fe.ClearValue(UIElement::RenderTransformProperty());
    }
}

static bool WalkFindInlinePercent(DependencyObject const& node, int depth = 0) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost()) {
            bool wasVertical = sp.Orientation() == Orientation::Vertical;
            if (wasVertical) sp.Orientation(Orientation::Horizontal);
            int spN = VisualTreeHelper::GetChildrenCount(sp);
            Wh_Log(L"[Battery] WalkFindInlinePercent: inner SP at depth %d, children=%d%s",
                depth, spN, wasVertical ? L" (was Vertical, forced Horizontal)" : L"");
            if (spN >= 2) {
                auto pct = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
                if (pct) {
                    g_batteryInlinePercentFE = pct;
                    ApplyOffset(pct, g_settings.inlinePercentX, g_settings.inlinePercentY);
                    Wh_Log(L"[Battery] Inline percent FE found (%s) and offset applied",
                        winrt::get_class_name(pct).c_str());
                }
            } else {
                Wh_Log(L"[Battery] Inner SP has %d children — not enough for percent", spN);
            }
            return true;
        }
        if (WalkFindInlinePercent(child, depth + 1)) return true;
    }
    if (depth == 0)
        Wh_Log(L"[Battery] WalkFindInlinePercent: no inner SP found in battery subtree");
    return false;
}

// Grid mode: force inner SP horizontal, widen glyph to gridSlotWidth so % lands in col 1.
static bool WalkFindGridPercent(DependencyObject const& node, int depth = 0) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost()) {
            if (sp.Orientation() == Orientation::Vertical) sp.Orientation(Orientation::Horizontal);
            g_batteryInnerPanel = sp;
            int spN = VisualTreeHelper::GetChildrenCount(sp);
            if (spN >= 1) {
                // Widen glyph to exactly one column width so % naturally lands at col 1 x-position.
                auto glyph = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
                if (glyph) glyph.Width((double)g_settings.gridSlotWidth);
            }
            if (spN >= 2) {
                auto pct = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
                if (pct) {
                    g_batteryInlinePercentFE = pct;
                    ApplyOffset(pct, g_settings.gridPercentX, g_settings.gridPercentY);
                    Wh_Log(L"[Grid] Battery %% positioned in col 1");
                }
            }
            return true;
        }
        if (WalkFindGridPercent(child, depth + 1)) return true;
    }
    return false;
}

static void SizeStackedBatteryRows(StackPanel const& innerSP) {
    int n = VisualTreeHelper::GetChildrenCount(innerSP);
    if (n >= 1) {
        auto glyph = VisualTreeHelper::GetChild(innerSP, 0).try_as<FrameworkElement>();
        if (glyph) {
            glyph.Width(32.0);
            glyph.Height(28.0);
            glyph.HorizontalAlignment(HorizontalAlignment::Center);
            ApplyOffset(glyph, g_settings.stackedBatteryX, g_settings.stackedBatteryY);
        }
    }
    if (n >= 2) {
        auto text = VisualTreeHelper::GetChild(innerSP, 1).try_as<FrameworkElement>();
        if (text) {
            text.HorizontalAlignment(HorizontalAlignment::Center);
            text.ClearValue(FrameworkElement::MarginProperty());
            ApplyOffset(text, g_settings.stackedPercentX, g_settings.stackedPercentY);
        }
    }
}

// Setting Height=NaN as a local value overrides template constraints (ClearValue would
// revert to the template default which may be 28px).
static void ClearHeightDescendants(DependencyObject const& node, int depth = 0) {
    if (depth > 8) return;
    auto fe = node.try_as<FrameworkElement>();
    if (fe) fe.Height(std::numeric_limits<double>::quiet_NaN());
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (child) ClearHeightDescendants(child, depth + 1);
    }
}

// ── OmniButton height helpers ─────────────────────────────────────────────

static void FreeOmniButtonHeight() {
    if (!g_omniButton) return;
    g_omniButton.Height(std::numeric_limits<double>::quiet_NaN());
    g_omniButton.InvalidateMeasure();
    auto parent = VisualTreeHelper::GetParent(g_omniButton);
    if (parent) {
        auto parentUI = parent.try_as<UIElement>();
        if (parentUI) parentUI.InvalidateMeasure();
        auto grandparent = VisualTreeHelper::GetParent(parent);
        if (grandparent) {
            auto gpUI = grandparent.try_as<UIElement>();
            if (gpUI) gpUI.InvalidateMeasure();
        }
    }
}

static void ApplyOmniButtonWidth() {
    if (!g_omniButton) return;

    double padding = static_cast<double>(g_settings.buttonHorizontalPadding);

    auto ctrl = g_omniButton.try_as<Control>();
    if (ctrl) {
        ctrl.Padding(Thickness{ padding, 0.0, padding, 0.0 });
        ctrl.HorizontalContentAlignment(HorizontalAlignment::Center);
        ctrl.VerticalContentAlignment(VerticalAlignment::Center);
    }

    if (g_settings.batteryMode == 1) {
        // Inline mode: battery slot has glyph + % text side by side, so it's
        // wider than 32px. Let the button auto-size to content + padding so
        // the hover highlight matches exactly and nothing is clipped.
        g_omniButton.ClearValue(FrameworkElement::WidthProperty());
        g_omniButton.ClearValue(FrameworkElement::MinWidthProperty());
    } else if (g_settings.batteryMode == 3) {
        // Grid mode: 2-column layout; width = 2 columns + padding.
        double width = g_settings.gridSlotWidth * 2.0 + padding * 2.0;
        g_omniButton.Width(width);
        g_omniButton.MinWidth(width);
        // Height: 2 rows of 28px (slots are the visible content; StackPanel reports 3*28=84 in layout
        // but transforms bring Volume and Battery into a 56px visual bounding box).
        g_omniButton.Height(56.0);
    } else {
        // Off/stacked: all slots are 32px wide, so fix the width explicitly.
        double width = 32.0 + padding * 2.0;
        g_omniButton.Width(width);
        g_omniButton.MinWidth(width);
    }

    // Prevent the button from stretching if its grid column is wider than needed.
    g_omniButton.HorizontalAlignment(HorizontalAlignment::Left);
    g_omniButton.InvalidateMeasure();
}

// ── XAML cleanup ─────────────────────────────────────────────────────────

// Clears all locally-set properties from elements we modified.
// Safe to call with null parameters; each block is try-caught independently.
static void CleanupXamlElements(
    StackPanel       sp,
    FrameworkElement btn,
    FrameworkElement wifi,
    FrameworkElement vol,
    FrameworkElement bp,
    StackPanel       bip,
    FrameworkElement bipct)
{
    try {
        if (sp) {
            sp.Orientation(Orientation::Horizontal);
            sp.ClearValue(StackPanel::SpacingProperty());
            sp.ClearValue(FrameworkElement::VerticalAlignmentProperty());
            int n = VisualTreeHelper::GetChildrenCount(sp);
            for (int i = 0; i < n; i++) {
                auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                if (child) {
                    child.ClearValue(FrameworkElement::WidthProperty());
                    child.ClearValue(FrameworkElement::HeightProperty());
                    child.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                    child.ClearValue(UIElement::RenderTransformProperty());
                    auto cp = child.try_as<ContentPresenter>();
                    if (cp) cp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                }
            }
        }
    } catch (...) {}
    try {
        if (btn) {
            btn.ClearValue(FrameworkElement::WidthProperty());
            btn.ClearValue(FrameworkElement::MinWidthProperty());
            btn.ClearValue(FrameworkElement::HeightProperty());
            btn.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
            btn.InvalidateMeasure();
            auto ctrl = btn.try_as<Control>();
            if (ctrl) {
                ctrl.ClearValue(Control::PaddingProperty());
                ctrl.ClearValue(Control::HorizontalContentAlignmentProperty());
                ctrl.ClearValue(Control::VerticalContentAlignmentProperty());
            }
        }
    } catch (...) {}
    try { if (wifi)  wifi.ClearValue(UIElement::RenderTransformProperty());  } catch (...) {}
    try { if (vol)   vol.ClearValue(UIElement::RenderTransformProperty());   } catch (...) {}
    try { if (bipct) bipct.ClearValue(UIElement::RenderTransformProperty()); } catch (...) {}
    try {
        if (bp) {
            bp.ClearValue(FrameworkElement::HeightProperty());
            bp.ClearValue(UIElement::RenderTransformProperty());
        }
    } catch (...) {}
    try {
        if (bip) {
            bip.Orientation(Orientation::Horizontal);  // ClearValue restores to Vertical (template default); must set explicitly
            bip.ClearValue(StackPanel::SpacingProperty());
            int bipN = VisualTreeHelper::GetChildrenCount(bip);
            for (int i = 0; i < bipN; i++) {
                auto fe = VisualTreeHelper::GetChild(bip, i).try_as<FrameworkElement>();
                if (fe) {
                    fe.ClearValue(FrameworkElement::WidthProperty());
                    fe.ClearValue(FrameworkElement::HeightProperty());
                    fe.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                    fe.ClearValue(FrameworkElement::MarginProperty());
                    fe.ClearValue(UIElement::RenderTransformProperty());
                }
            }
        }
    } catch (...) {}
}

static void ResetElementRefs() {
    g_omniStackPanel         = nullptr;
    g_omniButton             = nullptr;
    g_wifiPresenter          = nullptr;
    g_volumePresenter        = nullptr;
    g_batteryPresenter       = nullptr;
    g_batteryInnerPanel      = nullptr;
    g_batteryInlinePercentFE = nullptr;
}

static void RevokeLayoutUpdated() {
    if (g_layoutUpdatedSP && g_layoutUpdatedToken.value) {
        g_layoutUpdatedSP.LayoutUpdated(g_layoutUpdatedToken);
        g_layoutUpdatedToken = {};
    }
    g_layoutUpdatedSP = nullptr;
}

static void CleanupAndResetCurrentElements() {
    RevokeLayoutUpdated();

    auto sp    = g_omniStackPanel;
    auto btn   = g_omniButton;
    auto wifi  = g_wifiPresenter;
    auto vol   = g_volumePresenter;
    auto bp    = g_batteryPresenter;
    auto bip   = g_batteryInnerPanel;
    auto bipct = g_batteryInlinePercentFE;

    ResetElementRefs();
    CleanupXamlElements(sp, btn, wifi, vol, bp, bip, bipct);
}

// ── Layout application ────────────────────────────────────────────────────

static void ApplyLayout(StackPanel const& sp) {
    if (g_omniStackPanel) return;
    if (!sp.IsItemsHost()) return;
    // g_omniButton is set by ApplyAllSettings before this call

    g_omniStackPanel = sp;
    sp.Orientation(Orientation::Vertical);
    sp.VerticalAlignment(VerticalAlignment::Center);
    sp.Spacing(0);

    if (g_omniButton) {
        ApplyOmniButtonWidth();
        if (g_settings.batteryMode == 2)
            FreeOmniButtonHeight();
    }

    int n = VisualTreeHelper::GetChildrenCount(sp);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
        if (child) {
            child.Width(32.0);
            child.Height(28.0);
            child.HorizontalAlignment(HorizontalAlignment::Center);
            auto cp = child.try_as<ContentPresenter>();
            if (cp) cp.HorizontalContentAlignment(HorizontalAlignment::Center);
            if (i == 0) { g_wifiPresenter   = child; ApplyOffset(child, WifiX(),   WifiY());   }
            if (i == 1) { g_volumePresenter  = child; ApplyOffset(child, VolumeX(), VolumeY()); }
        }
    }

    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (HasBatteryDescendant(child)) {
            g_batteryPresenter = child;
            Wh_Log(L"[Battery] Battery slot at index %d (mode=%d)", i, g_settings.batteryMode);
            if (g_settings.batteryMode == 1) {
                child.Width(std::numeric_limits<double>::quiet_NaN());
                child.Height(28.0);
                ApplyOffset(child, g_settings.inlineBatteryX, g_settings.inlineBatteryY);
                WalkFindInlinePercent(child);
            } else if (g_settings.batteryMode == 2) {
                child.Width(std::numeric_limits<double>::quiet_NaN());
                ClearHeightDescendants(child);
                FlipBatteryLayout(child);
                if (g_batteryInnerPanel) {
                    g_batteryInnerPanel.Spacing(0.0);
                    SizeStackedBatteryRows(g_batteryInnerPanel);
                }
            } else if (g_settings.batteryMode == 3) {
                // Grid mode: battery moves from slot 2 (Y=56) to row 1 (Y=28).
                // Volume already moved to row 0 via VolumeX()/VolumeY() transforms.
                child.Width(std::numeric_limits<double>::quiet_NaN());
                child.Height(28.0);
                ApplyOffset(child, g_settings.gridBatteryX, -28 + g_settings.gridBatteryY);
                WalkFindGridPercent(child);
            } else {
                // Off mode: outer slot position unchanged; push % text off-screen without
                // touching inner panel orientation so the battery glyph renders normally.
                ApplyOffset(child, g_settings.offBatteryX, g_settings.offBatteryY);
                if (WalkFindInnerSP(child) && g_batteryInnerPanel) {
                    int bspN = VisualTreeHelper::GetChildrenCount(g_batteryInnerPanel);
                    if (bspN >= 2) {
                        auto text = VisualTreeHelper::GetChild(g_batteryInnerPanel, 1).try_as<FrameworkElement>();
                        if (text) ApplyOffset(text, g_settings.offPercentX, g_settings.offPercentY);
                    }
                }
            }
            break;
        }
    }
    if (!g_batteryPresenter)
        Wh_Log(L"[Battery] No battery slot found (desktop without battery?)");

    Wh_Log(L"[Layout] Applied vertical layout (children=%d)", n);
}

// ── Taskbar window helpers ─────────────────────────────────────────────────

static HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows([](HWND hWnd, LPARAM lParam) -> BOOL {
        DWORD pid;
        WCHAR cls[32];
        if (GetWindowThreadProcessId(hWnd, &pid) && pid == GetCurrentProcessId() &&
            GetClassName(hWnd, cls, ARRAYSIZE(cls)) &&
            _wcsicmp(cls, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(lParam) = hWnd;
            return FALSE;
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    return result;
}

using RunFromWindowThreadProc_t = void (*)(void*);

static bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, void* procParam) {
    static const UINT kMsg = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);

    struct Param { RunFromWindowThreadProc_t proc; void* procParam; };

    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (!dwThreadId) return false;
    if (dwThreadId == GetCurrentThreadId()) { proc(procParam); return true; }

    HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (nCode == HC_ACTION) {
            const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
            if (cwp->message == kMsg) {
                auto* p = (Param*)cwp->lParam;
                p->proc(p->procParam);
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }, nullptr, dwThreadId);
    if (!hook) return false;

    Param param{ proc, procParam };
    SendMessage(hWnd, kMsg, 0, (LPARAM)&param);
    UnhookWindowsHookEx(hook);
    return true;
}

// ── GetTaskbarXamlRoot infrastructure ─────────────────────────────────────

using CTaskBand_GetTaskbarHost_t =
    void* (WINAPI*)(void* pThis, void* taskbarHostSharedPtr);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using TaskbarHost_FrameHeight_t = int (WINAPI*)(void* pThis);
TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;

using std__Ref_count_base__Decref_t = void (WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;

    void* taskBand = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    void* taskBandForSite = taskBand;
    for (int i = 0; *(void**)taskBandForSite != CTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) return nullptr;
        taskBandForSite = (void**)taskBandForSite + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite, taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] && !taskbarHostSharedPtr[1]) return nullptr;

    size_t offset = 0x10;
#if defined(_M_X64)
    {
        // 48:83EC 28 | sub rsp,28
        // 48:83C1 48 | add rcx,48
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    {
        // 7f2303d5 pacibsp
        // fd7bbfa9 stp     fp, lr, [sp, #-0x10]!
        // fd030091 mov     fp, sp
        // 080c41f8 ldr     x8, [x0, #0x10]!
        const DWORD* p = (const DWORD*)TaskbarHost_FrameHeight_Original;
        if (p[0] == 0xD503237F && (p[1] & 0xFFC07FFF) == 0xA9807BFD &&
            p[2] == 0x910003FD && (p[3] & 0xFFF00FE0) == 0xF8400C00)
            offset = (p[3] >> 12) & 0xFF;
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#else
#error "Unsupported architecture"
#endif

    auto* iunk = *(IUnknown**)((BYTE*)taskbarHostSharedPtr[0] + offset);
    FrameworkElement taskbarElem = nullptr;
    iunk->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(taskbarElem));
    auto result = taskbarElem ? taskbarElem.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

// ── XAML tree helpers ─────────────────────────────────────────────────────

static FrameworkElement EnumChildElements(
    FrameworkElement const& element,
    std::function<bool(FrameworkElement)> const& cb)
{
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && cb(child)) return child;
    }
    return nullptr;
}

static FrameworkElement FindChildByName(FrameworkElement const& element, PCWSTR name) {
    return EnumChildElements(element, [name](FrameworkElement fe) {
        return fe.Name() == name;
    });
}

static FrameworkElement FindChildByClassName(FrameworkElement const& element, PCWSTR className) {
    return EnumChildElements(element, [className](FrameworkElement fe) {
        return winrt::get_class_name(fe) == className;
    });
}

static FrameworkElement FindChildRecursive(FrameworkElement const& element,
    std::function<bool(FrameworkElement)> const& cb, int maxDepth = 20)
{
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n && maxDepth > 0; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (cb(child)) return child;
        auto found = FindChildRecursive(child, cb, maxDepth - 1);
        if (found) return found;
    }
    return nullptr;
}

// ── Apply settings ────────────────────────────────────────────────────────

static void OnLayoutUpdated(IInspectable const&, IInspectable const&) {
    auto sp = g_layoutUpdatedSP;
    if (!sp) return;

    // Size wifi/volume slots if they arrived after initial ApplyLayout.
    if (!g_wifiPresenter || !g_volumePresenter) {
        int nc = VisualTreeHelper::GetChildrenCount(sp);
        for (int i = 0; i < nc; i++) {
            auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
            if (!child) continue;
            child.Width(32.0);
            child.Height(28.0);
            child.HorizontalAlignment(HorizontalAlignment::Center);
            auto cp = child.try_as<ContentPresenter>();
            if (cp) cp.HorizontalContentAlignment(HorizontalAlignment::Center);
            if (i == 0 && !g_wifiPresenter)  { g_wifiPresenter  = child; ApplyOffset(child, WifiX(),   WifiY());   }
            if (i == 1 && !g_volumePresenter) { g_volumePresenter = child; ApplyOffset(child, VolumeX(), VolumeY()); }
        }
    }

    bool needsBatteryFind = !g_batteryPresenter;
    bool needsFlip = ((g_settings.batteryMode == 2 || g_settings.batteryMode == 0) &&
                      g_batteryPresenter && !g_batteryInnerPanel) ||
                     (g_settings.batteryMode == 3 && g_batteryPresenter && !g_batteryInlinePercentFE);

    if (!needsBatteryFind && !needsFlip && g_wifiPresenter && g_volumePresenter) {
        sp.LayoutUpdated(g_layoutUpdatedToken);
        g_layoutUpdatedToken = {};
        g_layoutUpdatedSP = nullptr;
        return;
    }

    if (needsBatteryFind) {
        int nc = VisualTreeHelper::GetChildrenCount(sp);
        for (int i = 0; i < nc; i++) {
            auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
            if (!child) continue;
            if (HasBatteryDescendant(child)) {
                g_batteryPresenter = child;
                child.Width(std::numeric_limits<double>::quiet_NaN());
                child.HorizontalAlignment(HorizontalAlignment::Center);
                if (g_settings.batteryMode == 1) {
                    child.Height(28.0);
                    ApplyOffset(child, g_settings.inlineBatteryX, g_settings.inlineBatteryY);
                    WalkFindInlinePercent(child);
                } else if (g_settings.batteryMode == 2) {
                    child.Height(std::numeric_limits<double>::quiet_NaN());
                    ClearHeightDescendants(child);
                } else {
                    child.Height(28.0);
                    ApplyOffset(child, g_settings.offBatteryX, g_settings.offBatteryY);
                }
                Wh_Log(L"[Layout] Deferred battery slot found at index %d", i);
                break;
            }
        }
    }

    if (needsFlip && g_batteryPresenter) {
        if (g_settings.batteryMode == 2 && !g_batteryInnerPanel) {
            FlipBatteryLayout(g_batteryPresenter);
            if (g_batteryInnerPanel) {
                g_batteryInnerPanel.Spacing(0.0);
                SizeStackedBatteryRows(g_batteryInnerPanel);
                FreeOmniButtonHeight();
                Wh_Log(L"[Layout] Deferred battery inner StackPanel flipped (stacked)");
            }
        } else if (g_settings.batteryMode == 0 && !g_batteryInnerPanel) {
            if (WalkFindInnerSP(g_batteryPresenter) && g_batteryInnerPanel) {
                int bspN = VisualTreeHelper::GetChildrenCount(g_batteryInnerPanel);
                if (bspN >= 2) {
                    auto text = VisualTreeHelper::GetChild(g_batteryInnerPanel, 1).try_as<FrameworkElement>();
                    if (text) ApplyOffset(text, g_settings.offPercentX, g_settings.offPercentY);
                }
                Wh_Log(L"[Layout] Deferred battery %% text hidden (off mode)");
            }
        } else if (g_settings.batteryMode == 3 && !g_batteryInlinePercentFE) {
            WalkFindGridPercent(g_batteryPresenter);
            Wh_Log(L"[Layout] Deferred grid battery %% positioned");
        }
    }
}

static void ApplyAllSettings() {
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) { Wh_Log(L"[Apply] No taskbar window found"); return; }
    g_taskbarWnd = hTaskbarWnd;

    auto xamlRoot = GetTaskbarXamlRoot(hTaskbarWnd);
    if (!xamlRoot) { Wh_Log(L"[Apply] GetTaskbarXamlRoot failed"); return; }

    auto content = xamlRoot.Content().try_as<FrameworkElement>();
    if (!content) { Wh_Log(L"[Apply] XamlRoot content not a FrameworkElement"); return; }

    if (!g_omniStackPanel) {
        auto omniButton = FindChildRecursive(content, [](FrameworkElement fe) {
            return fe.Name() == L"ControlCenterButton";
        });
        if (omniButton) {
            g_omniButton = omniButton;
            FrameworkElement fe   = omniButton;
            FrameworkElement grid = FindChildByClassName(fe, L"Windows.UI.Xaml.Controls.Grid");
            FrameworkElement cp   = grid ? FindChildByName(grid, L"ContentPresenter") : nullptr;
            FrameworkElement ip   = cp   ? FindChildByClassName(cp, L"Windows.UI.Xaml.Controls.ItemsPresenter") : nullptr;
            auto sp = ip ? FindChildByClassName(ip, L"Windows.UI.Xaml.Controls.StackPanel").try_as<StackPanel>() : nullptr;
            if (sp && sp.IsItemsHost()) {
                ApplyLayout(sp);
                bool needsDeferred = !g_wifiPresenter || !g_volumePresenter ||
                                     !g_batteryPresenter ||
                                     ((g_settings.batteryMode == 2 || g_settings.batteryMode == 0) && !g_batteryInnerPanel) ||
                                     (g_settings.batteryMode == 3 && !g_batteryInlinePercentFE);
                if (needsDeferred) {
                    g_layoutUpdatedSP = sp;
                    g_layoutUpdatedToken = sp.LayoutUpdated(OnLayoutUpdated);
                    Wh_Log(L"[Apply] Registered LayoutUpdated for deferred slot sizing");
                }
            } else {
                Wh_Log(L"[Apply] IsItemsHost StackPanel not found under OmniButton");
            }
        } else {
            Wh_Log(L"[Apply] ControlCenterButton not found in XAML tree");
        }
    }
}

static void ApplyAllSettingsOnWindowThread() {
    HWND hTaskbarWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) return;
    RunFromWindowThread(hTaskbarWnd, [](void*) { ApplyAllSettings(); }, nullptr);
}

// ── IconView constructor hook ──────────────────────────────────────────────

using IconView_IconView_t = void*(WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* ret = IconView_IconView_Original(pThis);

    FrameworkElement iconView = nullptr;
    ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                           winrt::put_abi(iconView));
    if (!iconView) return ret;

    g_autoRevokerList.emplace_back();
    auto autoRevokerIt = std::prev(g_autoRevokerList.end());
    *autoRevokerIt = iconView.Loaded(
        winrt::auto_revoke_t{},
        [autoRevokerIt](IInspectable const&, RoutedEventArgs const&) {
            g_autoRevokerList.erase(autoRevokerIt);
            if (!g_unloading && !g_omniStackPanel)
                ApplyAllSettings();
        });

    return ret;
}

// ── System tray module detection and hook setup ───────────────────────────

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

static std::atomic<bool> g_systemTrayModuleHooked = false;

static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE hModule, UINT* puPtrLen) {
    void* pFixedFileInfo = nullptr;
    UINT uPtrLen = 0;
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (hResource) {
        HGLOBAL hGlobal = LoadResource(hModule, hResource);
        if (hGlobal) {
            void* pData = LockResource(hGlobal);
            if (pData) {
                if (!VerQueryValue(pData, L"\\", &pFixedFileInfo, &uPtrLen) || uPtrLen == 0) {
                    pFixedFileInfo = nullptr;
                    uPtrLen = 0;
                }
            }
        }
    }
    if (puPtrLen) *puPtrLen = uPtrLen;
    return (VS_FIXEDFILEINFO*)pFixedFileInfo;
}

// Order matters: SystemTray.dll is the new home (Win11 Insider 26200+);
// older builds have the symbols in Taskbar.View.dll.
static HMODULE GetSystemTrayModuleHandle() {
    HMODULE module = GetModuleHandleW(L"SystemTray.dll");
    if (!module) {
        module = GetModuleHandleW(L"Taskbar.View.dll");
        if (module) {
            // Starting with Taskbar.View.dll 2604.x, the SystemTray types moved
            // out into SystemTray.dll — don't hook this version.
            VS_FIXEDFILEINFO* fi = GetModuleVersionInfo(module, nullptr);
            WORD moduleMajor = fi ? HIWORD(fi->dwFileVersionMS) : 0;
            if (!moduleMajor || moduleMajor >= 2604) {
                Wh_Log(L"[Hooks] Skipping Taskbar.View.dll version %d", moduleMajor);
                module = nullptr;
            }
        }
    }
    if (!module)
        module = GetModuleHandleW(L"ExplorerExtensions.dll");
    return module;
}

static bool HookSystemTraySymbols(HMODULE hModule) {
    // SystemTray.dll, Taskbar.View.dll
    WindhawkUtils::SYMBOL_HOOK hooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original, IconView_IconView_Hook,
    }};
    if (!WindhawkUtils::HookSymbols(hModule, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"[Hooks] HookSymbols failed");
        return false;
    }
    return true;
}

static void HandleLoadedModuleIfSystemTray(HMODULE hModule, LPCWSTR lpLibFileName) {
    if (!g_systemTrayModuleHooked && GetSystemTrayModuleHandle() == hModule &&
        !g_systemTrayModuleHooked.exchange(true)) {
        Wh_Log(L"[LoadLib] %s — hooking symbols", lpLibFileName);
        if (HookSystemTraySymbols(hModule))
            Wh_ApplyHookOperations();
    }
}

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (hModule && lpLibFileName)
        HandleLoadedModuleIfSystemTray(hModule, lpLibFileName);
    return hModule;
}

// ── Symbol hook setup ─────────────────────────────────────────────────────

static bool HookTaskbarDllSymbols() {
    HMODULE hTaskbar = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!hTaskbar) { Wh_Log(L"[Hooks] Failed to load taskbar.dll"); return false; }

    // taskbar.dll
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {
            {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
            &CTaskBand_ITaskListWndSite_vftable,
        },
        {
            {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
            &CTaskBand_GetTaskbarHost_Original,
        },
        {
            {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
            &TaskbarHost_FrameHeight_Original,
        },
        {
            {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
            &std__Ref_count_base__Decref_Original,
        },
    };
    return WindhawkUtils::HookSymbols(hTaskbar, hooks, ARRAYSIZE(hooks));
}

// ── Windhawk lifecycle ─────────────────────────────────────────────────────

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Vertical OmniButton v1.4");
    LoadSettings();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll symbol hooks failed — continuing without GetTaskbarXamlRoot");

    if (HMODULE hSystemTray = GetSystemTrayModuleHandle()) {
        g_systemTrayModuleHooked = true;
        if (!HookSystemTraySymbols(hSystemTray))
            Wh_Log(L"[Init] System tray symbol hooks failed");
    } else {
        Wh_Log(L"[Init] System tray module not loaded yet");
        HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
        auto pLoadLibraryExW = kernelbase
            ? reinterpret_cast<LoadLibraryExW_t>(GetProcAddress(kernelbase, "LoadLibraryExW"))
            : nullptr;
        if (pLoadLibraryExW)
            WindhawkUtils::Wh_SetFunctionHookT(pLoadLibraryExW,
                                               LoadLibraryExW_Hook,
                                               &LoadLibraryExW_Original);
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_systemTrayModuleHooked) {
        if (HMODULE hSystemTray = GetSystemTrayModuleHandle()) {
            if (!g_systemTrayModuleHooked.exchange(true)) {
                Wh_Log(L"[AfterInit] System tray module found — hooking symbols");
                if (HookSystemTraySymbols(hSystemTray))
                    Wh_ApplyHookOperations();
            }
        }
    }
    if (g_systemTrayModuleHooked)
        ApplyAllSettingsOnWindowThread();
    Wh_Log(L"[AfterInit] systemTrayModuleHooked=%d", (int)g_systemTrayModuleHooked.load());
    // Initial application covers the hot-inject case (XAML tree already ready).
    // The IconView::IconView hook + auto-revoke Loaded subscription covers startup:
    // it fires for each new IconView construction and applies when the element loads.
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd)
        RunFromWindowThread(hWnd, [](void*) { CleanupAndResetCurrentElements(); }, nullptr);
    else
        CleanupAndResetCurrentElements();
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Settings] No taskbar window found"); return; }

    // On the UI thread: clean up existing XAML properties, reset globals, re-apply from scratch.
    RunFromWindowThread(hWnd, [](void*) {
        CleanupAndResetCurrentElements();
        ApplyAllSettings();
    }, nullptr);
}
