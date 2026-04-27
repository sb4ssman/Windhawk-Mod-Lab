// ==WindhawkMod==
// @id              vertical-omnibutton
// @name            Vertical OmniButton
// @description     Stacks Windows 11 wifi/volume/battery OmniButton vertically
// @version         1.1
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject
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

Hooks into the `Taskbar.View.dll` system tray implementation via symbol-based
function hooks. When OmniButton elements appear, the mod forces
`Orientation=Vertical` on the inner StackPanel and positions each icon slot
according to your settings.

## Usage

After enabling the mod, **restart explorer.exe** via Task Manager if icons don't appear immediately.

## Settings

Default offsets are tuned for a non-standard Windows 11 taskbar (two rows of taskbar, three rows 
of system-tray) in the Windhawk ecosystem. Use the per-mode offsets to align icons for your theme, scaling,
or taskbar layout.

- **Battery percentage** — Off / Inline / Stacked. 
- **Icon offsets** — each battery mode (Off / Inline / Stacked) has its own
  X/Y offsets for wifi, volume, battery, and percent. Settings are labeled by mode.

## Windows 11 Taskbar Styler compatibility

This mod does not use the Windows XAML Diagnostics API, so it is compatible
with the Windows 11 Taskbar Styler out of the box — no special settings required.

This mod actually grew out of a Taskbar Styler `style.yaml` config — a
[style.yaml is included in this repo](https://github.com/sb4ssman/Windhawk-Vertical-OmniButton/blob/main/style.yaml)
if you want to achieve the vertical layout through the Styler alone instead.
For most users the dedicated mod is the better choice: it handles battery percentage modes and dynamic
element timing that simple XAML styling cannot.

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
  $description: "Off: battery icon only.\nInline: percentage shown in the battery icon slot (3rd row).\nStacked: percentage as a separate 4th row below the battery icon.\n\nDefaults are tuned for a non-standard Windows 11 taskbar (two rows of taskbar, three rows of system-tray) in the Windhawk ecosystem. Use the per-mode offsets below to align icons for your theme, scaling, or taskbar layout. Suggestions welcome — open an issue or PR on GitHub."
  $options:
    - "off": "Off — battery icon only"
    - "inline": "Inline — percentage in battery slot (3rd row)"
    - "stacked": "Stacked — percentage as 4th row below battery"
- wifiOffX: -2
  $name: "Off mode: Wifi X"
  $description: "Wifi horizontal offset when battery % is Off. Negative = left, positive = right."
- wifiOffY: 0
  $name: "Off mode: Wifi Y"
  $description: "Wifi vertical offset when battery % is Off. Negative = up, positive = down."
- volumeOffX: 0
  $name: "Off mode: Volume X"
  $description: "Volume horizontal offset when battery % is Off. Negative = left, positive = right."
- volumeOffY: 0
  $name: "Off mode: Volume Y"
  $description: "Volume vertical offset when battery % is Off. Negative = up, positive = down."
- batteryOffX: 2
  $name: "Off mode: Battery X"
  $description: "Battery icon horizontal offset when battery % is Off. Negative = left, positive = right."
- batteryOffY: 0
  $name: "Off mode: Battery Y"
  $description: "Battery icon vertical offset when battery % is Off. Negative = up, positive = down."
- wifiInlineX: -2
  $name: "Inline mode: Wifi X"
  $description: "Wifi horizontal offset in Inline mode. Negative = left, positive = right."
- wifiInlineY: 0
  $name: "Inline mode: Wifi Y"
  $description: "Wifi vertical offset in Inline mode. Negative = up, positive = down."
- volumeInlineX: 0
  $name: "Inline mode: Volume X"
  $description: "Volume horizontal offset in Inline mode. Negative = left, positive = right."
- volumeInlineY: 0
  $name: "Inline mode: Volume Y"
  $description: "Volume vertical offset in Inline mode. Negative = up, positive = down."
- batteryInlineX: 2
  $name: "Inline mode: Battery X"
  $description: "Battery slot horizontal offset in Inline mode. Negative = left, positive = right. Default: 2."
- batteryInlineY: 0
  $name: "Inline mode: Battery Y"
  $description: "Battery slot vertical offset in Inline mode. Negative = up, positive = down."
- batteryInlinePercentX: 2
  $name: "Inline mode: Battery percent X"
  $description: "Percentage text horizontal offset within the inline battery slot. Negative = left, positive = right."
- batteryInlinePercentY: -1
  $name: "Inline mode: Battery percent Y"
  $description: "Percentage text vertical offset within the inline battery slot. Negative = up, positive = down."
- wifiStackedX: -2
  $name: "Stacked mode: Wifi X"
  $description: "Wifi horizontal offset in Stacked mode. Negative = left, positive = right."
- wifiStackedY: 7
  $name: "Stacked mode: Wifi Y"
  $description: "Wifi vertical offset in Stacked mode. Negative = up, positive = down."
- volumeStackedX: 0
  $name: "Stacked mode: Volume X"
  $description: "Volume horizontal offset in Stacked mode. Negative = left, positive = right."
- volumeStackedY: 0
  $name: "Stacked mode: Volume Y"
  $description: "Volume vertical offset in Stacked mode. Negative = up, positive = down."
- batteryGlyphX: 8
  $name: "Stacked mode: Battery glyph X"
  $description: "Battery icon row horizontal offset in Stacked mode. Negative = left, positive = right. Default: 8."
- batteryGlyphY: -6
  $name: "Stacked mode: Battery glyph Y"
  $description: "Battery icon row vertical offset in Stacked mode. Negative = up, positive = down. Default: -6."
- batteryPercentX: 2
  $name: "Stacked mode: Battery percent X"
  $description: "Percentage row horizontal offset in Stacked mode. Negative = left, positive = right. Default: 2."
- batteryPercentY: -11
  $name: "Stacked mode: Battery percent Y"
  $description: "Percentage row vertical offset in Stacked mode. Negative = up, positive = down. Default: -11."
*/
// ==/WindhawkModSettings==

#include <functional>
#include <limits>
#include <winrt/base.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime

#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;
using winrt::Windows::Foundation::IInspectable;

// ── Settings ───────────────────────────────────────────────────────────────

struct {
    int  batteryMode;          // 0=off, 1=inline (3rd row), 2=stacked (4th row)
    int  wifiOffX,    wifiOffY;
    int  wifiInlineX, wifiInlineY;
    int  wifiStackedX,wifiStackedY;
    int  volumeOffX,    volumeOffY;
    int  volumeInlineX, volumeInlineY;
    int  volumeStackedX,volumeStackedY;
    int  batteryOffX,     batteryOffY;
    int  batteryGlyphX,   batteryGlyphY;
    int  batteryPercentX, batteryPercentY;
    int  batteryInlineX,  batteryInlineY;
    int  batteryInlinePercentX, batteryInlinePercentY;
} g_settings;

bool g_unloading = false;

void LoadSettings() {
    {
        auto* bm = Wh_GetStringSetting(L"batteryMode");
        if (bm) {
            if (wcscmp(bm, L"inline") == 0)       g_settings.batteryMode = 1;
            else if (wcscmp(bm, L"stacked") == 0) g_settings.batteryMode = 2;
            else                                   g_settings.batteryMode = 0;
            Wh_FreeStringSetting(bm);
        } else {
            g_settings.batteryMode = 0;
        }
    }
    auto clampOffset = [](int v) { return v < -20 ? -20 : v > 20 ? 20 : v; };
    g_settings.wifiOffX      = clampOffset(Wh_GetIntSetting(L"wifiOffX"));
    g_settings.wifiOffY      = clampOffset(Wh_GetIntSetting(L"wifiOffY"));
    g_settings.wifiInlineX   = clampOffset(Wh_GetIntSetting(L"wifiInlineX"));
    g_settings.wifiInlineY   = clampOffset(Wh_GetIntSetting(L"wifiInlineY"));
    g_settings.wifiStackedX  = clampOffset(Wh_GetIntSetting(L"wifiStackedX"));
    g_settings.wifiStackedY  = clampOffset(Wh_GetIntSetting(L"wifiStackedY"));
    g_settings.volumeOffX    = clampOffset(Wh_GetIntSetting(L"volumeOffX"));
    g_settings.volumeOffY    = clampOffset(Wh_GetIntSetting(L"volumeOffY"));
    g_settings.volumeInlineX = clampOffset(Wh_GetIntSetting(L"volumeInlineX"));
    g_settings.volumeInlineY = clampOffset(Wh_GetIntSetting(L"volumeInlineY"));
    g_settings.volumeStackedX= clampOffset(Wh_GetIntSetting(L"volumeStackedX"));
    g_settings.volumeStackedY= clampOffset(Wh_GetIntSetting(L"volumeStackedY"));
    g_settings.batteryOffX     = clampOffset(Wh_GetIntSetting(L"batteryOffX"));
    g_settings.batteryOffY     = clampOffset(Wh_GetIntSetting(L"batteryOffY"));
    g_settings.batteryGlyphX   = clampOffset(Wh_GetIntSetting(L"batteryGlyphX"));
    g_settings.batteryGlyphY   = clampOffset(Wh_GetIntSetting(L"batteryGlyphY"));
    g_settings.batteryPercentX = clampOffset(Wh_GetIntSetting(L"batteryPercentX"));
    g_settings.batteryPercentY = clampOffset(Wh_GetIntSetting(L"batteryPercentY"));
    g_settings.batteryInlineX  = clampOffset(Wh_GetIntSetting(L"batteryInlineX"));
    g_settings.batteryInlineY  = clampOffset(Wh_GetIntSetting(L"batteryInlineY"));
    g_settings.batteryInlinePercentX = clampOffset(Wh_GetIntSetting(L"batteryInlinePercentX"));
    g_settings.batteryInlinePercentY = clampOffset(Wh_GetIntSetting(L"batteryInlinePercentY"));
}

static int WifiX()   { return g_settings.batteryMode==1 ? g_settings.wifiInlineX   : g_settings.batteryMode==2 ? g_settings.wifiStackedX   : g_settings.wifiOffX;   }
static int WifiY()   { return g_settings.batteryMode==1 ? g_settings.wifiInlineY   : g_settings.batteryMode==2 ? g_settings.wifiStackedY   : g_settings.wifiOffY;   }
static int VolumeX() { return g_settings.batteryMode==1 ? g_settings.volumeInlineX : g_settings.batteryMode==2 ? g_settings.volumeStackedX : g_settings.volumeOffX; }
static int VolumeY() { return g_settings.batteryMode==1 ? g_settings.volumeInlineY : g_settings.batteryMode==2 ? g_settings.volumeStackedY : g_settings.volumeOffY; }

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

// ── Explorer restart ──────────────────────────────────────────────────────

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
        Wh_Log(L"[Battery4] No inner StackPanel found (% may not be in tree yet)");
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
                    ApplyOffset(pct, g_settings.batteryInlinePercentX, g_settings.batteryInlinePercentY);
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

static void SizeStackedBatteryRows(StackPanel const& innerSP) {
    int n = VisualTreeHelper::GetChildrenCount(innerSP);
    if (n >= 1) {
        auto glyph = VisualTreeHelper::GetChild(innerSP, 0).try_as<FrameworkElement>();
        if (glyph) {
            glyph.Width(32.0);
            glyph.Height(28.0);
            glyph.HorizontalAlignment(HorizontalAlignment::Center);
            ApplyOffset(glyph, g_settings.batteryGlyphX, g_settings.batteryGlyphY);
        }
    }
    if (n >= 2) {
        auto text = VisualTreeHelper::GetChild(innerSP, 1).try_as<FrameworkElement>();
        if (text) {
            text.HorizontalAlignment(HorizontalAlignment::Center);
            text.ClearValue(FrameworkElement::MarginProperty());
            ApplyOffset(text, g_settings.batteryPercentX, g_settings.batteryPercentY);
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

// ── Battery percentage (registry) ────────────────────────────────────────

static DWORD g_originalBatteryPercent = MAXDWORD;

static constexpr LPCWSTR kAdvancedKey =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";

static void ApplyBatteryPercent(int mode) {
    bool show = (mode > 0);
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kAdvancedKey, 0,
                      KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        Wh_Log(L"[Battery] Failed to open registry key");
        return;
    }
    if (g_originalBatteryPercent == MAXDWORD) {
        DWORD val = 0, sz = sizeof(val), type = 0;
        if (RegQueryValueExW(hKey, L"TaskbarBatteryPercent", nullptr, &type,
                             reinterpret_cast<BYTE*>(&val), &sz) == ERROR_SUCCESS)
            g_originalBatteryPercent = val;
        else
            g_originalBatteryPercent = 0;
    }
    DWORD val = show ? 1 : 0;
    RegSetValueExW(hKey, L"TaskbarBatteryPercent", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&val), sizeof(val));
    RegCloseKey(hKey);
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                       reinterpret_cast<LPARAM>(kAdvancedKey));
    Wh_Log(L"[Battery] TaskbarBatteryPercent set to %d (requires explorer restart)", val);
}

static void RestoreBatteryPercent() {
    if (g_originalBatteryPercent == MAXDWORD) return;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kAdvancedKey, 0,
                      KEY_WRITE, &hKey) != ERROR_SUCCESS) return;
    RegSetValueExW(hKey, L"TaskbarBatteryPercent", 0, REG_DWORD,
                   reinterpret_cast<const BYTE*>(&g_originalBatteryPercent),
                   sizeof(g_originalBatteryPercent));
    RegCloseKey(hKey);
    SendNotifyMessageW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                       reinterpret_cast<LPARAM>(kAdvancedKey));
    Wh_Log(L"[Battery] Restored TaskbarBatteryPercent to %lu", g_originalBatteryPercent);
    g_originalBatteryPercent = MAXDWORD;
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

static void RestoreOmniButtonHeight() {
    if (!g_omniButton) return;
    g_omniButton.ClearValue(FrameworkElement::HeightProperty());
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
            sp.ClearValue(StackPanel::OrientationProperty());
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
            btn.ClearValue(FrameworkElement::HeightProperty());
            btn.InvalidateMeasure();
            auto ctrl = btn.try_as<Control>();
            if (ctrl) {
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
        auto ctrl = g_omniButton.try_as<Control>();
        if (ctrl) {
            ctrl.HorizontalContentAlignment(HorizontalAlignment::Center);
            ctrl.VerticalContentAlignment(VerticalAlignment::Center);
        }
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
                ApplyOffset(child, g_settings.batteryInlineX, g_settings.batteryInlineY);
                WalkFindInlinePercent(child);
            } else if (g_settings.batteryMode == 2) {
                child.Width(std::numeric_limits<double>::quiet_NaN());
                ClearHeightDescendants(child);
                FlipBatteryLayout(child);
                if (g_batteryInnerPanel) {
                    g_batteryInnerPanel.Spacing(0.0);
                    SizeStackedBatteryRows(g_batteryInnerPanel);
                }
            } else {
                ApplyOffset(child, g_settings.batteryOffX, g_settings.batteryOffY);
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

    size_t offset = 0x48;
#if defined(_M_X64)
    {
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
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
    bool needsFlip = g_settings.batteryMode == 2 && g_batteryPresenter && !g_batteryInnerPanel;

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
                    ApplyOffset(child, g_settings.batteryInlineX, g_settings.batteryInlineY);
                    WalkFindInlinePercent(child);
                } else if (g_settings.batteryMode == 2) {
                    child.Height(std::numeric_limits<double>::quiet_NaN());
                    ClearHeightDescendants(child);
                } else {
                    child.Width(32.0); child.Height(28.0);
                    ApplyOffset(child, g_settings.batteryOffX, g_settings.batteryOffY);
                }
                Wh_Log(L"[Layout] Deferred battery slot found at index %d", i);
                break;
            }
        }
    }

    if (g_settings.batteryMode == 2 && g_batteryPresenter && !g_batteryInnerPanel) {
        FlipBatteryLayout(g_batteryPresenter);
        if (g_batteryInnerPanel) {
            g_batteryInnerPanel.Spacing(0.0);
            SizeStackedBatteryRows(g_batteryInnerPanel);
            FreeOmniButtonHeight();
            Wh_Log(L"[Layout] Deferred battery inner StackPanel flipped");
        }
    }
}

static void ApplyAllSettings() {
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) { Wh_Log(L"[Apply] No taskbar window found"); return; }

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
                                     (g_settings.batteryMode == 2 && !g_batteryInnerPanel);
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
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
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

    iconView.Loaded([](IInspectable const&, RoutedEventArgs const&) {
        if (!g_unloading && !g_omniStackPanel)
            ApplyAllSettings();
    });

    return ret;
}

// ── LoadLibraryExW hook ───────────────────────────────────────────────────

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;

static bool g_taskbarViewDllLoaded = false;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (hModule && lpLibFileName) {
        const wchar_t* base = wcsrchr(lpLibFileName, L'\\');
        base = base ? (base + 1) : lpLibFileName;
        if (!g_taskbarViewDllLoaded && _wcsicmp(base, L"Taskbar.View.dll") == 0) {
            Wh_Log(L"[LoadLib] Taskbar.View.dll loaded — hooking symbols");
            // Taskbar.View.dll
            WindhawkUtils::SYMBOL_HOOK hooks[] = {{
                {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
                &IconView_IconView_Original, IconView_IconView_Hook,
            }};
            if (WindhawkUtils::HookSymbols(hModule, hooks, ARRAYSIZE(hooks)))
                g_taskbarViewDllLoaded = true;
            else
                Wh_Log(L"[LoadLib] HookSymbols failed for Taskbar.View.dll");
        }
    }
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

static bool HookTaskbarViewDllSymbols(HMODULE hTaskbarView) {
    // Taskbar.View.dll
    WindhawkUtils::SYMBOL_HOOK hooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original, IconView_IconView_Hook,
    }};
    if (!WindhawkUtils::HookSymbols(hTaskbarView, hooks, ARRAYSIZE(hooks)))
        return false;
    g_taskbarViewDllLoaded = true;
    return true;
}

static HMODULE GetTaskbarViewModuleHandle() {
    static const WCHAR* names[] = { L"Taskbar.View.dll", L"taskbar.view.dll" };
    for (auto name : names) {
        HMODULE h = GetModuleHandleW(name);
        if (h) return h;
    }
    return nullptr;
}

// ── Windhawk lifecycle ─────────────────────────────────────────────────────

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Vertical OmniButton v1.3");
    LoadSettings();
    ApplyBatteryPercent(g_settings.batteryMode);

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll symbol hooks failed — continuing without GetTaskbarXamlRoot");

    if (HMODULE hTaskbarView = GetTaskbarViewModuleHandle()) {
        if (!HookTaskbarViewDllSymbols(hTaskbarView))
            Wh_Log(L"[Init] Taskbar.View.dll symbol hooks failed");
    } else {
        HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
        auto pLoadLibraryExW = kernelbase
            ? reinterpret_cast<LoadLibraryExW_t>(GetProcAddress(kernelbase, "LoadLibraryExW"))
            : nullptr;
        if (pLoadLibraryExW) {
            Wh_SetFunctionHook((void*)pLoadLibraryExW,
                                (void*)LoadLibraryExW_Hook,
                                (void**)&LoadLibraryExW_Original);
            Wh_Log(L"[Init] LoadLibraryExW hook queued (waiting for Taskbar.View.dll)");
        }
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_taskbarViewDllLoaded) {
        if (HMODULE hTaskbarView = GetTaskbarViewModuleHandle()) {
            Wh_Log(L"[AfterInit] Taskbar.View.dll found — hooking symbols");
            HookTaskbarViewDllSymbols(hTaskbarView);
        }
    }
    if (g_taskbarViewDllLoaded)
        ApplyAllSettingsOnWindowThread();
    Wh_Log(L"[AfterInit] taskbarViewLoaded=%d", (int)g_taskbarViewDllLoaded);
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    if (g_layoutUpdatedSP && g_layoutUpdatedToken.value) {
        g_layoutUpdatedSP.LayoutUpdated(g_layoutUpdatedToken);
        g_layoutUpdatedToken = {};
        g_layoutUpdatedSP = nullptr;
    }

    RestoreBatteryPercent();

    auto sp    = g_omniStackPanel;
    auto btn   = g_omniButton;
    auto wifi  = g_wifiPresenter;
    auto vol   = g_volumePresenter;
    auto bp    = g_batteryPresenter;
    auto bip   = g_batteryInnerPanel;
    auto bipct = g_batteryInlinePercentFE;
    g_omniStackPanel         = nullptr;
    g_omniButton             = nullptr;
    g_wifiPresenter          = nullptr;
    g_volumePresenter        = nullptr;
    g_batteryPresenter       = nullptr;
    g_batteryInnerPanel      = nullptr;
    g_batteryInlinePercentFE = nullptr;

    CleanupXamlElements(sp, btn, wifi, vol, bp, bip, bipct);

    // Also schedule async dispatch for wrong-thread cases.
    auto dispSrc = sp ? sp.try_as<FrameworkElement>() : nullptr;
    if (!dispSrc) return;
    auto disp = dispSrc.Dispatcher();
    if (!disp) return;

    HMODULE hSelf = nullptr;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        reinterpret_cast<LPCWSTR>(&Wh_ModUninit), &hSelf);

    try {
        auto _ = disp.RunAsync(
            winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
            [sp, btn, wifi, vol, bp, bip, bipct, hSelf]() {
                Wh_Log(L"[Uninit] Async cleanup running");
                CleanupXamlElements(sp, btn, wifi, vol, bp, bip, bipct);
                Wh_Log(L"[Uninit] Async cleanup done");
                if (hSelf) FreeLibrary(hSelf);
            });
    } catch (...) {
        if (hSelf) FreeLibrary(hSelf);
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");
    ApplyBatteryPercent(g_settings.batteryMode);

    HWND hWnd = FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Settings] No taskbar window found"); return; }

    // On the UI thread: clean up existing XAML properties, reset globals, re-apply from scratch.
    RunFromWindowThread(hWnd, [](void*) {
        // Revoke LayoutUpdated before resetting.
        if (g_layoutUpdatedSP && g_layoutUpdatedToken.value) {
            g_layoutUpdatedSP.LayoutUpdated(g_layoutUpdatedToken);
            g_layoutUpdatedToken = {};
            g_layoutUpdatedSP = nullptr;
        }
        // Clear XAML properties on whatever elements we currently hold.
        CleanupXamlElements(g_omniStackPanel, g_omniButton,
                            g_wifiPresenter, g_volumePresenter,
                            g_batteryPresenter, g_batteryInnerPanel,
                            g_batteryInlinePercentFE);
        // Reset all globals so ApplyAllSettings re-traverses from scratch.
        g_omniStackPanel         = nullptr;
        g_omniButton             = nullptr;
        g_wifiPresenter          = nullptr;
        g_volumePresenter        = nullptr;
        g_batteryPresenter       = nullptr;
        g_batteryInnerPanel      = nullptr;
        g_batteryInlinePercentFE = nullptr;
        // Re-apply.
        ApplyAllSettings();
    }, nullptr);
}
