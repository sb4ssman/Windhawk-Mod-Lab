// ==WindhawkMod==
// @id              vertical-omnibutton
// @name            OmniButton Customizer
// @description     Rearrange the Windows 11 wifi/volume/battery OmniButton into any grid layout with per-element nudging
// @version         2.0
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# OmniButton Customizer

Rearranges the Windows 11 system tray OmniButton (wifi, volume/sound, battery) from its
default horizontal layout into any grid configuration you choose.

You control the grid: how many rows, how many columns, and the X/Y pixel offset of every
individual element. Battery percentage (if enabled in Windows) is treated as a fourth element.

![2x2 grid on a standard taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Vertical-OmniButton/main/screenshot-stacked.png)

## How it works

Hooks into the Windows 11 system tray implementation via symbol-based function hooks
(`SystemTray.dll` on newer builds, `Taskbar.View.dll` on older ones). When the OmniButton
loads, the mod forces `Orientation=Vertical` on its inner StackPanel and positions each
icon slot using RenderTransform offsets derived from the grid geometry.

## Settings

- **Slot width / height** — size of each grid cell in pixels. Height 0 = auto (taskbar height ÷ rows).
- **Grid columns / rows** — 0 = auto (computed from item count and taskbar height).
- **Short group alignment** — when the last row or column has fewer items than the others, align them to start, center, or end.
- **Button horizontal padding** — total OmniButton width = columns × slot width + 2 × padding.
- **Per-element nudge** — fine pixel offsets (X/Y) applied on top of the grid geometry for wifi, volume, battery icon, and battery percentage.

## Presets

### Standard 2×2 (default)
Wifi and volume top row, battery and percent bottom row.
`gridColumns: 2` · `fillOrder: rowFirst` · `itemOrder: "wifi volume battery percent"`

### Single column — 3 icons
Wifi, volume, battery stacked vertically (good when percent is disabled in Windows).
`gridColumns: 1` · `itemOrder: "wifi volume battery"`

### Single column — all 4 icons
All items stacked in one column (good for tall taskbars).
`gridColumns: 1` · `itemOrder: "wifi volume battery percent"`

### Swap wifi and volume
Volume top-left, wifi top-right.
`itemOrder: "volume wifi battery percent"`

### Side-by-side columns
Wifi and battery in the left column, volume and percent in the right (column-first fill).
`gridColumns: 2` · `fillOrder: columnFirst`

### Wide bar
All icons in one horizontal row — original OmniButton style.
`gridColumns: 0` · `gridRows: 1`

## Windows 11 Taskbar Styler compatibility

This mod does not use the Windows XAML Diagnostics API and is compatible with Windows 11
Taskbar Styler out of the box.

## Related mods

- [Taskbar height and icon size](https://windhawk.net/mods/taskbar-icon-size)
- [Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization)
- [Multirow taskbar for Windows 11](https://windhawk.net/mods/taskbar-multirow)
- [Taskbar tray icon spacing and grid](https://windhawk.net/mods/taskbar-notification-icon-spacing)
- [Windows 11 Taskbar Styler](https://windhawk.net/mods/windows-11-taskbar-styler)

*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- slotWidth: 32
  $name: Slot width (px)
  $description: >-
    Width of each grid column in pixels. Default 32 matches the standard 32px icon size.
    Total button width = columns × slot width + 2 × padding.

- slotHeight: 0
  $name: Slot height (px, 0 = auto)
  $description: >-
    Height of each grid row in pixels. 0 = auto: the mod reads the taskbar height
    and divides by the number of rows. Minimum auto height is 20px.

- gridColumns: 2
  $name: Grid columns (0 = auto)
  $description: >-
    Number of columns in the grid. 0 = auto: computed so all items fit given
    the taskbar height and row count. Default 2 gives a 2×2 layout for four items.

- gridRows: 0
  $name: Grid rows (0 = auto)
  $description: >-
    Number of rows in the grid. 0 = auto: derived from taskbar height ÷ slot height.
    Ignored when Grid columns is set to a value that fully determines the layout.

- fillOrder: rowFirst
  $name: Fill order
  $description: >-
    Whether items fill across rows first or down columns first.
  $options:
  - columnFirst: Column-first (top to bottom, then right)
  - rowFirst: Row-first (left to right, then down)

- shortGroupAlign: center
  $name: Short group alignment
  $description: >-
    When the last row or column has fewer items than the others, how to align them.
  $options:
  - start: Start (left/top)
  - center: Center
  - end: End (right/bottom)

- buttonHorizontalPadding: 2
  $name: Button horizontal padding (px)
  $description: >-
    Horizontal padding on each side of the icon grid. Total button width =
    columns × slot width + 2 × padding. Default 2.

- itemOrder: "wifi volume battery percent"
  $name: Item order
  $description: >-
    Space-separated list of icons in grid fill order. Names: wifi, volume, battery, percent.
    Rearrange tokens to change icon order. Items not present in Windows are skipped.
    Example: "volume wifi battery percent" puts volume before wifi.

- wifiX: 0
  $name: Wifi nudge X
  $description: Fine horizontal offset for the wifi icon. Negative = left.

- wifiY: 0
  $name: Wifi nudge Y
  $description: Fine vertical offset for the wifi icon. Negative = up.

- volumeX: 0
  $name: Volume nudge X
  $description: Fine horizontal offset for the volume icon.

- volumeY: 0
  $name: Volume nudge Y
  $description: Fine vertical offset for the volume icon.

- batteryX: 0
  $name: Battery nudge X
  $description: Fine horizontal offset for the battery icon.

- batteryY: 0
  $name: Battery nudge Y
  $description: Fine vertical offset for the battery icon.

- percentX: 0
  $name: Battery percent nudge X
  $description: Fine horizontal offset for the battery percentage text.

- percentY: 0
  $name: Battery percent nudge Y
  $description: Fine vertical offset for the battery percentage text.
*/
// ==/WindhawkModSettings==

#include <atomic>
#include <functional>
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

enum class FillOrder { ColumnFirst, RowFirst };
enum class ShortAlign { Start, Center, End };

struct {
    int        slotWidth;
    int        slotHeight;       // 0 = auto
    int        gridColumns;      // 0 = auto
    int        gridRows;         // 0 = auto
    FillOrder  fillOrder;
    ShortAlign shortGroupAlign;
    int        buttonHorizontalPadding;
    wchar_t    itemOrderStr[128];
    int        wifiX,    wifiY;
    int        volumeX,  volumeY;
    int        batteryX, batteryY;
    int        percentX, percentY;
} g_settings;

std::atomic<bool> g_unloading = false;

static void LoadSettings() {
    auto clampSlot    = [](int v) { return v < 0 ? 0 : v > 80 ? 80 : v; };
    auto clampGrid    = [](int v) { return v < 0 ? 0 : v > 8  ? 8  : v; };
    auto clampPad     = [](int v) { return v < 0 ? 0 : v > 24 ? 24 : v; };
    auto clampNudge   = [](int v) { return v < -40 ? -40 : v > 40 ? 40 : v; };

    int sw = clampSlot(Wh_GetIntSetting(L"slotWidth"));
    g_settings.slotWidth  = sw < 16 ? 32 : sw;
    g_settings.slotHeight = clampSlot(Wh_GetIntSetting(L"slotHeight"));
    g_settings.gridColumns = clampGrid(Wh_GetIntSetting(L"gridColumns"));
    g_settings.gridRows    = clampGrid(Wh_GetIntSetting(L"gridRows"));
    g_settings.buttonHorizontalPadding = clampPad(Wh_GetIntSetting(L"buttonHorizontalPadding"));

    {
        auto* s = Wh_GetStringSetting(L"fillOrder");
        g_settings.fillOrder = (s && wcscmp(s, L"rowFirst") == 0) ? FillOrder::RowFirst : FillOrder::ColumnFirst;
        if (s) Wh_FreeStringSetting(s);
    }
    {
        auto* s = Wh_GetStringSetting(L"shortGroupAlign");
        if (s && wcscmp(s, L"start") == 0)     g_settings.shortGroupAlign = ShortAlign::Start;
        else if (s && wcscmp(s, L"end") == 0)  g_settings.shortGroupAlign = ShortAlign::End;
        else                                    g_settings.shortGroupAlign = ShortAlign::Center;
        if (s) Wh_FreeStringSetting(s);
    }

    {
        auto* s = Wh_GetStringSetting(L"itemOrder");
        if (s && *s) {
            wcsncpy(g_settings.itemOrderStr, s, 127);
            g_settings.itemOrderStr[127] = L'\0';
            Wh_FreeStringSetting(s);
        } else {
            if (s) Wh_FreeStringSetting(s);
            wcscpy(g_settings.itemOrderStr, L"wifi volume battery percent");
        }
    }

    g_settings.wifiX    = clampNudge(Wh_GetIntSetting(L"wifiX"));
    g_settings.wifiY    = clampNudge(Wh_GetIntSetting(L"wifiY"));
    g_settings.volumeX  = clampNudge(Wh_GetIntSetting(L"volumeX"));
    g_settings.volumeY  = clampNudge(Wh_GetIntSetting(L"volumeY"));
    g_settings.batteryX = clampNudge(Wh_GetIntSetting(L"batteryX"));
    g_settings.batteryY = clampNudge(Wh_GetIntSetting(L"batteryY"));
    g_settings.percentX = clampNudge(Wh_GetIntSetting(L"percentX"));
    g_settings.percentY = clampNudge(Wh_GetIntSetting(L"percentY"));
}

// ── Cached element references ─────────────────────────────────────────────

static StackPanel       g_omniStackPanel{ nullptr };
static FrameworkElement g_omniButton{ nullptr };
static FrameworkElement g_wifiPresenter{ nullptr };
static FrameworkElement g_volumePresenter{ nullptr };
static FrameworkElement g_batteryPresenter{ nullptr };
static StackPanel       g_batteryInnerPanel{ nullptr };
static FrameworkElement g_batteryGlyphFE{ nullptr };
static FrameworkElement g_batteryPercentFE{ nullptr };

static StackPanel         g_layoutUpdatedSP{ nullptr };
static winrt::event_token g_layoutUpdatedToken{};

static HWND g_taskbarWnd = nullptr;

static std::list<FrameworkElement::Loaded_revoker> g_autoRevokerList;

// ── Grid geometry ─────────────────────────────────────────────────────────
//
// Items are: 0=wifi, 1=volume, 2=battery, 3=percent (optional).
// The IsItemsHost StackPanel is vertical; each child is a 28px slot stacked top-to-bottom.
// We use RenderTransform (TranslateTransform) to move each slot to its grid cell.
// Item i at grid position (col, row) gets offset:
//   X = col * slotWidth
//   Y = -(i * slotHeight) + row * slotHeight   (un-does slot's natural Y, places at row)
//
// slotHeight defaults to taskbar_height / rows (read at apply time via GetWindowRect).

struct GridGeom {
    int cols;
    int rows;
    int slotH;   // resolved slot height (px)
};

static GridGeom ResolveGeometry(int itemCount, HWND hTaskbarWnd) {
    GridGeom g{};

    // Measure taskbar height
    int taskbarH = 48;
    if (hTaskbarWnd) {
        RECT r{};
        if (GetWindowRect(hTaskbarWnd, &r))
            taskbarH = r.bottom - r.top;
    }

    // Determine rows first (drives slot height)
    int rows = g_settings.gridRows;
    if (rows <= 0) {
        if (g_settings.slotHeight > 0) {
            rows = taskbarH / g_settings.slotHeight;
            if (rows < 1) rows = 1;
        } else {
            // Auto: fit as many 32px rows as the taskbar allows, capped at itemCount.
            // When rows == itemCount, cols auto-computes to 1 (single column for tall taskbars).
            rows = taskbarH / 32;
            if (rows > itemCount) rows = itemCount;
            if (rows < 2) rows = 2;
        }
    }

    // Resolve slot height — cap at 32px so icons don't look tiny in oversized slots.
    int slotH = g_settings.slotHeight > 0 ? g_settings.slotHeight : (taskbarH / rows);
    if (slotH > 32) slotH = 32;
    if (slotH < 16) slotH = 16;

    // Determine columns
    int cols = g_settings.gridColumns;
    if (cols <= 0) {
        // Auto: enough columns to fit all items in the resolved rows
        cols = (itemCount + rows - 1) / rows;
        if (cols < 1) cols = 1;
    }

    g.rows  = rows;
    g.cols  = cols;
    g.slotH = slotH;
    return g;
}

// Returns (col, row) for item index given fill order and geometry.
static void ItemGridPos(int idx, int cols, int rows, FillOrder fill,
                        ShortAlign align, int itemCount,
                        int& outCol, int& outRow)
{
    if (fill == FillOrder::ColumnFirst) {
        // Fill top-to-bottom, then right
        int fullCols = itemCount / rows;       // columns that are full
        int remainder = itemCount % rows;      // items in the last partial column
        outCol = idx / rows;
        outRow = idx % rows;
        // Short column alignment for the last column (if partial)
        if (remainder > 0 && outCol == fullCols) {
            int gap = rows - remainder;
            if (align == ShortAlign::Center) outRow += gap / 2;
            else if (align == ShortAlign::End) outRow += gap;
        }
    } else {
        // Fill left-to-right, then down
        int fullRows = itemCount / cols;
        int remainder = itemCount % cols;
        outRow = idx / cols;
        outCol = idx % cols;
        if (remainder > 0 && outRow == fullRows) {
            int gap = cols - remainder;
            if (align == ShortAlign::Center) outCol += gap / 2;
            else if (align == ShortAlign::End) outCol += gap;
        }
    }
}

// ── XAML helpers ──────────────────────────────────────────────────────────

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

// Finds the inner StackPanel inside the battery ContentPresenter, forces it Horizontal,
// widens the glyph child to slotWidth so the % text lands at offset slotW within the SP.
// Sets g_batteryInnerPanel and g_batteryPercentFE. Does NOT apply offset — caller does that.
static bool WalkSetupBatteryInnerPanel(DependencyObject const& node, int slotW, int depth = 0) {
    if (depth > 5) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost()) {
            if (sp.Orientation() == Orientation::Vertical)
                sp.Orientation(Orientation::Horizontal);
            g_batteryInnerPanel = sp;
            int spN = VisualTreeHelper::GetChildrenCount(sp);
            if (spN >= 1) {
                auto glyph = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
                if (glyph) {
                    g_batteryGlyphFE = glyph;
                    glyph.Width(static_cast<double>(slotW));
                    glyph.VerticalAlignment(VerticalAlignment::Center);
                }
            }
            if (spN >= 2) {
                auto pct = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
                if (pct) {
                    g_batteryPercentFE = pct;
                    pct.VerticalAlignment(VerticalAlignment::Center);
                    Wh_Log(L"[Battery] %% element found (children=%d)", spN);
                }
            } else {
                Wh_Log(L"[Battery] Inner SP has %d children — no %% element (percent disabled?)", spN);
            }
            return true;
        }
        if (WalkSetupBatteryInnerPanel(child, slotW, depth + 1)) return true;
    }
    return false;
}

// ── OmniButton sizing ─────────────────────────────────────────────────────

static void ApplyOmniButtonSize(const GridGeom& geom) {
    if (!g_omniButton) return;
    double pad = static_cast<double>(g_settings.buttonHorizontalPadding);
    double w   = geom.cols * g_settings.slotWidth + pad * 2.0;
    double h   = geom.rows * geom.slotH;

    auto ctrl = g_omniButton.try_as<Control>();
    if (ctrl) {
        ctrl.Padding(Thickness{ pad, 0.0, pad, 0.0 });
        ctrl.HorizontalContentAlignment(HorizontalAlignment::Left);
        ctrl.VerticalContentAlignment(VerticalAlignment::Center);
    }
    g_omniButton.Width(w);
    g_omniButton.MinWidth(w);
    g_omniButton.Height(h);
    g_omniButton.HorizontalAlignment(HorizontalAlignment::Left);
    g_omniButton.InvalidateMeasure();
}

// ── XAML cleanup ──────────────────────────────────────────────────────────

static void CleanupXamlElements(
    StackPanel       sp,
    FrameworkElement btn,
    FrameworkElement wifi,
    FrameworkElement vol,
    FrameworkElement bp,
    StackPanel       bip,
    FrameworkElement biglyph,
    FrameworkElement bipct)
{
    try {
        if (sp) {
            sp.Orientation(Orientation::Horizontal);
            sp.ClearValue(StackPanel::SpacingProperty());
            sp.ClearValue(FrameworkElement::VerticalAlignmentProperty());  // reverts to template default (Stretch)
            int n = VisualTreeHelper::GetChildrenCount(sp);
            for (int i = 0; i < n; i++) {
                auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                if (child) {
                    child.ClearValue(FrameworkElement::WidthProperty());
                    child.ClearValue(FrameworkElement::HeightProperty());
                    child.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                    child.ClearValue(UIElement::RenderTransformProperty());
                    auto cp = child.try_as<ContentPresenter>();
                    if (cp) {
                        cp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                        cp.ClearValue(ContentPresenter::VerticalContentAlignmentProperty());
                    }
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
    try {
        if (bipct) {
            bipct.ClearValue(UIElement::RenderTransformProperty());
            bipct.ClearValue(FrameworkElement::WidthProperty());
            bipct.ClearValue(FrameworkElement::VerticalAlignmentProperty());
            auto tb = bipct.try_as<TextBlock>();
            if (tb) tb.ClearValue(TextBlock::TextAlignmentProperty());
        }
    } catch (...) {}
    try {
        if (bp) {
            bp.ClearValue(FrameworkElement::WidthProperty());
            bp.ClearValue(FrameworkElement::HeightProperty());
            bp.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
            bp.ClearValue(UIElement::RenderTransformProperty());
            auto bcp = bp.try_as<ContentPresenter>();
            if (bcp) {
                bcp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                bcp.ClearValue(ContentPresenter::VerticalContentAlignmentProperty());
            }
        }
    } catch (...) {}
    try {
        if (bip) {
            bip.Orientation(Orientation::Horizontal);
            bip.ClearValue(StackPanel::SpacingProperty());
            int bipN = VisualTreeHelper::GetChildrenCount(bip);
            for (int i = 0; i < bipN; i++) {
                auto fe = VisualTreeHelper::GetChild(bip, i).try_as<FrameworkElement>();
                if (fe) {
                    fe.ClearValue(FrameworkElement::WidthProperty());
                    fe.ClearValue(FrameworkElement::HeightProperty());
                    fe.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
                    fe.ClearValue(FrameworkElement::VerticalAlignmentProperty());
                    fe.ClearValue(FrameworkElement::MarginProperty());
                    fe.ClearValue(UIElement::RenderTransformProperty());
                    auto tb = fe.try_as<TextBlock>();
                    if (tb) tb.ClearValue(TextBlock::TextAlignmentProperty());
                }
            }
        }
    } catch (...) {}
    // Direct cleanup on stored glyph reference — belt-and-suspenders in case bip loop missed it
    try {
        if (biglyph) {
            biglyph.ClearValue(FrameworkElement::WidthProperty());
            biglyph.ClearValue(FrameworkElement::VerticalAlignmentProperty());
            biglyph.ClearValue(UIElement::RenderTransformProperty());
        }
    } catch (...) {}
    // Force a synchronous layout pass so cleared properties take effect immediately
    try { if (btn) btn.UpdateLayout(); } catch (...) {}
}

static void ResetElementRefs() {
    g_omniStackPanel    = nullptr;
    g_omniButton        = nullptr;
    g_wifiPresenter     = nullptr;
    g_volumePresenter   = nullptr;
    g_batteryPresenter  = nullptr;
    g_batteryInnerPanel = nullptr;
    g_batteryGlyphFE    = nullptr;
    g_batteryPercentFE  = nullptr;
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
    auto sp      = g_omniStackPanel;
    auto btn     = g_omniButton;
    auto wifi    = g_wifiPresenter;
    auto vol     = g_volumePresenter;
    auto bp      = g_batteryPresenter;
    auto bip     = g_batteryInnerPanel;
    auto biglyph = g_batteryGlyphFE;
    auto bipct   = g_batteryPercentFE;
    ResetElementRefs();
    CleanupXamlElements(sp, btn, wifi, vol, bp, bip, biglyph, bipct);
}

// ── Layout application ────────────────────────────────────────────────────

// Resolves grid position indices for all four items given which items are present
// and the user-configured itemOrder string.
// posMap[item] = grid position (0-based, among present items only); -1 for absent items.
// item indices: 0=wifi, 1=volume, 2=battery, 3=percent
static void ResolveItemPositions(const wchar_t* orderStr, bool hasBattery, bool hasPercent,
                                  int posMap[4])
{
    const bool present[4] = {true, true, hasBattery, hasPercent};
    const wchar_t* const names[4] = {L"wifi", L"volume", L"battery", L"percent"};
    bool assigned[4] = {};
    int nextPos = 0;

    const wchar_t* p = orderStr;
    while (*p) {
        while (*p == L' ' || *p == L'\t' || *p == L',') p++;
        if (!*p) break;
        const wchar_t* start = p;
        while (*p && *p != L' ' && *p != L'\t' && *p != L',') p++;
        size_t len = (size_t)(p - start);
        for (int i = 0; i < 4; i++) {
            if (!assigned[i] && present[i] &&
                wcslen(names[i]) == len && wcsncmp(start, names[i], len) == 0)
            {
                posMap[i] = nextPos++;
                assigned[i] = true;
                break;
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        if (present[i] && !assigned[i])
            posMap[i] = nextPos++;
    }
    for (int i = 0; i < 4; i++) {
        if (!present[i]) posMap[i] = -1;
    }
}

// When shortGroupAlign == Center and an item is in the short row/column, integer grid math
// gives col += gap/2 which is 0 for gap=1 (one missing item). This function returns a
// pixel nudge (half a slot) to visually center the item in the remaining space.
static void ShortGroupCenterNudge(int gridPos, int cols, int rows, FillOrder fill,
                                  int itemCount, int slotW, int slotH,
                                  int& nudgeX, int& nudgeY) {
    nudgeX = nudgeY = 0;
    if (fill == FillOrder::RowFirst) {
        int fullRows = itemCount / cols;
        int remainder = itemCount % cols;
        if (remainder > 0 && (gridPos / cols) == fullRows) {
            int gap = cols - remainder;
            if (gap % 2 == 1) nudgeX = slotW / 2;
        }
    } else {
        int fullCols = itemCount / rows;
        int remainder = itemCount % rows;
        if (remainder > 0 && (gridPos / rows) == fullCols) {
            int gap = rows - remainder;
            if (gap % 2 == 1) nudgeY = slotH / 2;
        }
    }
}

// Positions a single slot (child of IsItemsHost SP) at grid cell (col, row).
// slotIdx is the child's natural index in the SP (0-based, used to compute Y un-offset).
static void PositionSlot(FrameworkElement const& fe, int slotIdx,
                         int col, int row, const GridGeom& geom,
                         int nudgeX, int nudgeY)
{
    // Natural Y of slot[i] in a vertical SP with uniform heights is i * slotH.
    // We want it at row * slotH instead.
    int baseX = col * g_settings.slotWidth;
    int baseY = -(slotIdx * geom.slotH) + (row * geom.slotH);
    ApplyOffset(fe, baseX + nudgeX, baseY + nudgeY);
}

static void ApplyLayout(StackPanel const& sp, HWND hTaskbarWnd) {
    if (g_omniStackPanel) return;
    if (!sp.IsItemsHost()) return;

    g_omniStackPanel = sp;
    sp.Orientation(Orientation::Vertical);
    sp.VerticalAlignment(VerticalAlignment::Top);
    sp.Spacing(0);

    int n = VisualTreeHelper::GetChildrenCount(sp);

    // Find battery slot (typically index 2, but search to be safe)
    int battIdx = -1;
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
        if (child && HasBatteryDescendant(child)) { battIdx = i; break; }
    }

    bool hasBatteryPresenter = battIdx >= 0;
    bool hasPercent = false;

    if (hasBatteryPresenter) {
        g_batteryPresenter = VisualTreeHelper::GetChild(sp, battIdx).try_as<FrameworkElement>();
        // Try to find the inner panel — detect if % is in tree
        if (WalkSetupBatteryInnerPanel(g_batteryPresenter, g_settings.slotWidth)) {
            hasPercent = (g_batteryPercentFE != nullptr);
        }
    }

    int itemCount = hasBatteryPresenter ? (hasPercent ? 4 : 3) : 2;
    GridGeom geom = ResolveGeometry(itemCount, hTaskbarWnd);

    // Resolve grid position for each item from the itemOrder string
    int posMap[4];
    ResolveItemPositions(g_settings.itemOrderStr, hasBatteryPresenter, hasPercent, posMap);

    Wh_Log(L"[Layout] items=%d cols=%d rows=%d slotH=%d order=[%d,%d,%d,%d]",
        itemCount, geom.cols, geom.rows, geom.slotH,
        posMap[0], posMap[1], posMap[2], posMap[3]);

    // Size the OmniButton
    if (g_omniButton) ApplyOmniButtonSize(geom);

    // Resolve (col, row) for a grid position and compute short-group center nudge.
    auto resolvePos = [&](int gridPos, int& col, int& row, int& cnx, int& cny) {
        ItemGridPos(gridPos, geom.cols, geom.rows,
                    g_settings.fillOrder, g_settings.shortGroupAlign,
                    itemCount, col, row);
        cnx = cny = 0;
        if (g_settings.shortGroupAlign == ShortAlign::Center) {
            ShortGroupCenterNudge(gridPos, geom.cols, geom.rows, g_settings.fillOrder,
                                  itemCount, g_settings.slotWidth, geom.slotH, cnx, cny);
        }
    };

    // Position each slot.
    // HorizontalAlignment::Left keeps transform math simple (baseline x=0).
    // HorizontalContentAlignment::Center centers the icon *within* each slot.
    // Item 0 = wifi
    if (n >= 1) {
        auto wifi = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
        if (wifi) {
            g_wifiPresenter = wifi;
            wifi.Width(static_cast<double>(g_settings.slotWidth));
            wifi.Height(static_cast<double>(geom.slotH));
            wifi.HorizontalAlignment(HorizontalAlignment::Left);
            auto cp = wifi.try_as<ContentPresenter>();
            if (cp) {
                cp.HorizontalContentAlignment(HorizontalAlignment::Center);
                cp.VerticalContentAlignment(VerticalAlignment::Center);
            }
            int col, row, cnx, cny;
            resolvePos(posMap[0], col, row, cnx, cny);
            PositionSlot(wifi, 0, col, row, geom, g_settings.wifiX + cnx, g_settings.wifiY + cny);
        }
    }
    // Item 1 = volume
    if (n >= 2) {
        auto vol = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
        if (vol) {
            g_volumePresenter = vol;
            vol.Width(static_cast<double>(g_settings.slotWidth));
            vol.Height(static_cast<double>(geom.slotH));
            vol.HorizontalAlignment(HorizontalAlignment::Left);
            auto cp = vol.try_as<ContentPresenter>();
            if (cp) {
                cp.HorizontalContentAlignment(HorizontalAlignment::Center);
                cp.VerticalContentAlignment(VerticalAlignment::Center);
            }
            int col, row, cnx, cny;
            resolvePos(posMap[1], col, row, cnx, cny);
            PositionSlot(vol, 1, col, row, geom, g_settings.volumeX + cnx, g_settings.volumeY + cny);
        }
    }
    // Item 2 = battery. Width spans as many columns as battery+percent need so the inner SP's
    // natural width doesn't make the IsItemsHost SP wider than the grid (which would shift all icons).
    if (hasBatteryPresenter && g_batteryPresenter) {
        double battW = static_cast<double>(g_settings.slotWidth * (hasPercent ? 2 : 1));
        g_batteryPresenter.Width(battW);
        g_batteryPresenter.Height(static_cast<double>(geom.slotH));
        g_batteryPresenter.HorizontalAlignment(HorizontalAlignment::Left);
        auto bcp = g_batteryPresenter.try_as<ContentPresenter>();
        if (bcp) bcp.VerticalContentAlignment(VerticalAlignment::Center);
        int col, row, cnx, cny;
        resolvePos(posMap[2], col, row, cnx, cny);
        PositionSlot(g_batteryPresenter, battIdx, col, row, geom,
                     g_settings.batteryX + cnx, g_settings.batteryY + cny);
    }
    // Item 3 = battery percent (inside battery CP's inner SP, offset relative to battery position).
    // Glyph width = slotWidth, so percent naturally sits at x=slotWidth within the inner SP.
    if (hasPercent && g_batteryPresenter && g_batteryPercentFE) {
        int bCol, bRow, bcnx, bcny;
        resolvePos(posMap[2], bCol, bRow, bcnx, bcny);
        int pCol, pRow, pcnx, pcny;
        resolvePos(posMap[3], pCol, pRow, pcnx, pcny);
        // Percent is at x=slotWidth within inner SP relative to battery CP's left edge.
        // relX moves it to the correct column: (pCol - bCol - 1) * slotWidth.
        // relY moves it to the correct row offset from battery.
        int relX = (pCol - bCol - 1) * g_settings.slotWidth + g_settings.percentX + (pcnx - bcnx);
        int relY = (pRow - bRow) * geom.slotH + g_settings.percentY + (pcny - bcny);
        ApplyOffset(g_batteryPercentFE, relX, relY);
        Wh_Log(L"[Layout] pct at grid(%d,%d) battery at grid(%d,%d) relX=%d relY=%d",
            pCol, pRow, bCol, bRow, relX, relY);
    }

    Wh_Log(L"[Layout] Applied grid layout (SP children=%d)", n);
}

// ── Deferred layout (LayoutUpdated) ──────────────────────────────────────

static void OnLayoutUpdated(IInspectable const&, IInspectable const&) {
    auto sp = g_layoutUpdatedSP;
    if (!sp) return;

    bool allReady = g_wifiPresenter && g_volumePresenter &&
                    (!g_batteryPresenter || g_batteryInnerPanel);
    if (allReady) {
        sp.LayoutUpdated(g_layoutUpdatedToken);
        g_layoutUpdatedToken = {};
        g_layoutUpdatedSP = nullptr;
        return;
    }

    // Check if the tree has grown since the initial apply — if something new is present,
    // do a full re-apply so geometry and offsets are computed with complete item count.
    bool batteryNowPresent = false;
    if (!g_batteryPresenter) {
        int nc = VisualTreeHelper::GetChildrenCount(sp);
        for (int i = 0; i < nc; i++) {
            auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
            if (child && HasBatteryDescendant(child)) { batteryNowPresent = true; break; }
        }
    }
    bool percentNowPresent = g_batteryPresenter && !g_batteryInnerPanel &&
                             WalkSetupBatteryInnerPanel(g_batteryPresenter, g_settings.slotWidth);

    if (!batteryNowPresent && !percentNowPresent) return;  // still waiting

    Wh_Log(L"[Layout] Deferred: battery=%d percent=%d — re-applying full layout",
        (int)batteryNowPresent, (int)percentNowPresent);

    sp.LayoutUpdated(g_layoutUpdatedToken);
    g_layoutUpdatedToken = {};
    g_layoutUpdatedSP = nullptr;

    auto savedOmniButton = g_omniButton;
    CleanupAndResetCurrentElements();
    g_omniButton = savedOmniButton;
    ApplyLayout(sp, g_taskbarWnd);
}

// ── Taskbar and window thread helpers ─────────────────────────────────────

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
    if (!CTaskBand_GetTaskbarHost_Original || !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original)
        return nullptr;

    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;

    void* taskBand = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    if (!taskBand) return nullptr;
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
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else
            Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    {
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
    if (!iunk) {
        std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }
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

static FrameworkElement FindChildByClassName(FrameworkElement const& element, PCWSTR className) {
    return EnumChildElements(element, [className](FrameworkElement fe) {
        return winrt::get_class_name(fe) == className;
    });
}

static FrameworkElement FindChildByName(FrameworkElement const& element, PCWSTR name) {
    return EnumChildElements(element, [name](FrameworkElement fe) {
        return fe.Name() == name;
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

static void ApplyAllSettings() {
    HWND hTaskbarWnd = FindCurrentProcessTaskbarWnd();
    if (!hTaskbarWnd) { Wh_Log(L"[Apply] No taskbar window found"); return; }
    g_taskbarWnd = hTaskbarWnd;

    try {
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
                    ApplyLayout(sp, hTaskbarWnd);
                    bool needsDeferred = !g_wifiPresenter || !g_volumePresenter ||
                                         !g_batteryPresenter ||
                                         (g_batteryPresenter && !g_batteryInnerPanel);
                    if (needsDeferred) {
                        g_layoutUpdatedSP = sp;
                        g_layoutUpdatedToken = sp.LayoutUpdated(OnLayoutUpdated);
                        Wh_Log(L"[Apply] Registered LayoutUpdated for deferred elements");
                    }
                } else {
                    Wh_Log(L"[Apply] IsItemsHost StackPanel not found under OmniButton");
                }
            } else {
                Wh_Log(L"[Apply] ControlCenterButton not found in XAML tree");
            }
        }
    } catch (...) {
        Wh_Log(L"[Apply] Exception during injection (XAML not ready)");
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

static HMODULE GetSystemTrayModuleHandle() {
    HMODULE module = GetModuleHandleW(L"SystemTray.dll");
    if (!module) {
        module = GetModuleHandleW(L"Taskbar.View.dll");
        if (module) {
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
    Wh_Log(L"[Init] OmniButton Customizer v2.0");
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
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(hWnd, [](void*) {
            g_autoRevokerList.clear();
            CleanupAndResetCurrentElements();
        }, nullptr);
    } else {
        g_autoRevokerList.clear();
        CleanupAndResetCurrentElements();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Settings] No taskbar window found"); return; }

    RunFromWindowThread(hWnd, [](void*) {
        CleanupAndResetCurrentElements();
        ApplyAllSettings();
    }, nullptr);
}
