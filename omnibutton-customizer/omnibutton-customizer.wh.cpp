// ==WindhawkMod==
// @id              omnibutton-customizer
// @name            OmniButton Customizer
// @description     Rearrange the Windows 11 OmniButton (wifi/volume/battery) into any grid — custom order, nudges, independent battery+percent placement, and per-glyph colors with animation
// @version         1.0
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# OmniButton Customizer

Rearranges the Windows 11 system tray OmniButton (wifi, volume/sound, battery, battery
percentage) into any grid layout. Designed for multi-row taskbars.

## Battery / percent modes

**Coupled** (default): battery and percent share a slot, rendered side-by-side inside the
native inner panel. Works best when both are in adjacent grid cells.

**Independent**: battery glyph and percent text are each treated as independent grid items
and can be placed at any grid position, including non-adjacent ones. The battery
ContentPresenter spans the full grid footprint and both sub-elements are offset absolutely.

## Grid settings

- **Slot width / height** — size of each grid cell. Height 0 = taskbar height ÷ rows.
- **Grid columns / rows** — 0 = auto: a single column when all items fit the
  taskbar height (double-height taskbars), otherwise more columns — 4 items on
  a single-height taskbar become a 2x2.
- **Fill order** — row-first or column-first.
- **Short group alignment** — when the last row/column is shorter, align start/center/end.

## Item order

`itemOrder` is a space-separated list: `wifi`, `volume`, `battery`, `percent`.
Rearrange tokens to change which grid cell each item lands in. Items absent from
Windows (e.g. no battery on a desktop) are silently skipped.

## Per-item glyph colors

Set `wifiColor`, `volumeColor`, `batteryColor`, `percentColor` to a hex color
(`#RRGGBB` or `#AARRGGBB`, the alpha byte is honored), the generics `accent`,
`accentLight`, and `accentDark` for the Windows accent shades, or `transparent`.
Leave empty to use the default theme color.

## Animated colors

Set `wifiColorTo` (and the matching `wifiColor` as the starting color) to make the
wifi glyph animate between the two colors in a looping pulse. Repeat for any item.
`colorAnimateDuration` controls the half-cycle duration in milliseconds.

## Presets

### Standard 2×2
`gridColumns: 2` · `fillOrder: rowFirst` · `itemOrder: "wifi volume battery percent"`
(the auto default already picks this shape on a single-height taskbar)

### Single column — 3 icons (no percent)
`gridColumns: 1` · `itemOrder: "wifi volume battery"`

### Single column — all 4 icons
`gridColumns: 1` · `itemOrder: "wifi volume battery percent"`

### Percent top, battery bottom (independent mode)
`batteryPercentMode: independent` · `itemOrder: "wifi volume percent battery"`

### Wide bar (original OmniButton style)
`gridRows: 1` · `itemOrder: "wifi volume battery"`

## Windows 11 Taskbar Styler compatibility

Does not use XAML Diagnostics. Compatible with Windows 11 Taskbar Styler.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- itemOrder: "wifi volume battery percent"
  $name: Item order
  $description: >-
    Space-separated tokens in grid fill order: wifi, volume, battery, percent.
    Absent items (no battery on desktop) are skipped automatically.

- batteryPercentMode: coupled
  $name: Battery / percent mode
  $description: >-
    Coupled: percent renders inside the battery slot (native behavior).
    Independent: battery glyph and percent are independent grid items and can be
    placed anywhere — even non-adjacent cells.
  $options:
  - coupled: Coupled
  - independent: Independent

- gridColumns: 0
  $name: Grid columns (0 = auto)
  $description: >-
    Columns in the layout grid. 0 (default) picks automatically: a single
    column when all items fit the taskbar height (double-height taskbars),
    otherwise more columns — 4 items on a single-height taskbar become a 2x2.

- gridRows: 0
  $name: Grid rows (0 = auto)
  $description: >-
    Rows in the layout grid. 0 = automatic from the item count and taskbar
    height (see Grid columns).

- fillOrder: rowFirst
  $name: Fill order
  $options:
  - rowFirst: Row-first (left to right, then down)
  - columnFirst: Column-first (top to bottom, then right)

- shortGroupAlign: center
  $name: Short group alignment
  $description: Alignment of the last row/column when it has fewer items than the others.
  $options:
  - start: Start (left/top)
  - center: Center
  - end: End (right/bottom)

- slotWidth: 32
  $name: Slot width (px)
  $description: >-
    Width of each grid column in pixels. Default 32 matches the standard icon size.

- slotHeight: 0
  $name: Slot height (px, 0 = auto)
  $description: >-
    Height of each grid row. 0 = taskbar height ÷ rows (minimum 20px, capped at 32px).

- buttonHorizontalPadding: 2
  $name: Button horizontal padding (px)
  $description: Internal horizontal padding on each side of the grid. Default 2.

- wifiOffsetX: 0
  $name: Wifi nudge X
  $description: Fine horizontal offset for the wifi glyph. Negative = left.

- wifiOffsetY: 0
  $name: Wifi nudge Y
  $description: Fine vertical offset for the wifi glyph. Negative = up.

- volumeOffsetX: 0
  $name: Volume nudge X
- volumeOffsetY: 0
  $name: Volume nudge Y

- batteryOffsetX: 0
  $name: Battery nudge X
- batteryOffsetY: 0
  $name: Battery nudge Y

- percentOffsetX: 0
  $name: Battery percent nudge X
- percentOffsetY: 0
  $name: Battery percent nudge Y

- wifiColor: ""
  $name: Wifi icon color
  $description: >-
    Hex ("#RRGGBB" or "#AARRGGBB"), "accent" / "accentLight" / "accentDark",
    or "transparent". Empty = theme default.

- wifiColorTo: ""
  $name: Wifi icon animated color
  $description: >-
    If set, the wifi glyph pulses between Wifi icon color and this color.
    Requires Wifi icon color to be set.

- volumeColor: ""
  $name: Volume icon color
  $description: >-
    Hex, "accent" / "accentLight" / "accentDark", or "transparent".
    Empty = theme default.

- volumeColorTo: ""
  $name: Volume icon animated color

- batteryColor: ""
  $name: Battery icon color
  $description: >-
    Hex, "accent" / "accentLight" / "accentDark", or "transparent".
    Empty = theme default.

- batteryColorTo: ""
  $name: Battery icon animated color

- percentColor: ""
  $name: Battery percent color
  $description: >-
    Hex, "accent" / "accentLight" / "accentDark", or "transparent".
    Empty = theme default.

- percentColorTo: ""
  $name: Battery percent animated color

- colorAnimateDuration: 2000
  $name: Color animation half-cycle (ms)
  $description: >-
    Duration of one half of the color animation cycle in milliseconds.
    The animation auto-reverses, so total cycle = 2× this value. Default 2000.
*/
// ==/WindhawkModSettings==

#include <algorithm>
#include <atomic>
#include <functional>
#include <list>
#include <vector>
#include <winrt/base.h>
#include <windhawk_api.h>
#include <windhawk_utils.h>

#undef GetCurrentTime

#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;
using namespace winrt::Windows::UI::Xaml::Media::Animation;
using winrt::Windows::UI::Color;
using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::Foundation::TimeSpan;

// ── Settings ───────────────────────────────────────────────────────────────

enum class FillOrder     { ColumnFirst, RowFirst };
enum class ShortAlign    { Start, Center, End };
enum class BattPctMode   { Coupled, Independent };

struct {
    int        slotWidth;
    int        slotHeight;
    int        gridColumns;
    int        gridRows;
    FillOrder  fillOrder;
    ShortAlign shortGroupAlign;
    int        buttonHorizontalPadding;
    wchar_t    itemOrderStr[128];
    BattPctMode batteryPercentMode;
    int        wifiX,    wifiY;
    int        volumeX,  volumeY;
    int        batteryX, batteryY;
    int        percentX, percentY;
    wchar_t    wifiColor[32],     wifiColorTo[32];
    wchar_t    volumeColor[32],   volumeColorTo[32];
    wchar_t    batteryColor[32],  batteryColorTo[32];
    wchar_t    percentColor[32],  percentColorTo[32];
    int        colorAnimateDuration;
} g_settings;

std::atomic<bool> g_unloading = false;

static void LoadColorSetting(PCWSTR key, wchar_t (&buf)[32]) {
    auto* s = Wh_GetStringSetting(key);
    if (s && *s) { wcsncpy(buf, s, 31); buf[31] = L'\0'; }
    else          buf[0] = L'\0';
    if (s) Wh_FreeStringSetting(s);
}

static void LoadSettings() {
    auto clampSlot  = [](int v) { return v < 0 ? 0 : v > 80 ? 80 : v; };
    auto clampGrid  = [](int v) { return v < 0 ? 0 : v > 8  ? 8  : v; };
    auto clampPad   = [](int v) { return v < 0 ? 0 : v > 24 ? 24 : v; };
    auto clampNudge = [](int v) { return v < -40 ? -40 : v > 40 ? 40 : v; };

    int sw = clampSlot(Wh_GetIntSetting(L"slotWidth"));
    g_settings.slotWidth  = sw < 16 ? 32 : sw;
    g_settings.slotHeight = clampSlot(Wh_GetIntSetting(L"slotHeight"));
    g_settings.gridColumns = clampGrid(Wh_GetIntSetting(L"gridColumns"));
    g_settings.gridRows    = clampGrid(Wh_GetIntSetting(L"gridRows"));
    g_settings.buttonHorizontalPadding = clampPad(Wh_GetIntSetting(L"buttonHorizontalPadding"));

    { auto* s = Wh_GetStringSetting(L"fillOrder");
      g_settings.fillOrder = (s && wcscmp(s,L"rowFirst")==0) ? FillOrder::RowFirst : FillOrder::ColumnFirst;
      if (s) Wh_FreeStringSetting(s); }

    { auto* s = Wh_GetStringSetting(L"shortGroupAlign");
      if      (s && wcscmp(s,L"start")==0) g_settings.shortGroupAlign = ShortAlign::Start;
      else if (s && wcscmp(s,L"end")==0)   g_settings.shortGroupAlign = ShortAlign::End;
      else                                  g_settings.shortGroupAlign = ShortAlign::Center;
      if (s) Wh_FreeStringSetting(s); }

    { auto* s = Wh_GetStringSetting(L"itemOrder");
      if (s && *s) {
          wcsncpy(g_settings.itemOrderStr, s, 127); g_settings.itemOrderStr[127] = L'\0';
          Wh_FreeStringSetting(s);
      } else {
          if (s) Wh_FreeStringSetting(s);
          wcscpy(g_settings.itemOrderStr, L"wifi volume battery percent");
      } }

    { auto* s = Wh_GetStringSetting(L"batteryPercentMode");
      g_settings.batteryPercentMode = (s && wcscmp(s,L"independent")==0)
                                    ? BattPctMode::Independent : BattPctMode::Coupled;
      if (s) Wh_FreeStringSetting(s); }

    g_settings.wifiX    = clampNudge(Wh_GetIntSetting(L"wifiOffsetX"));
    g_settings.wifiY    = clampNudge(Wh_GetIntSetting(L"wifiOffsetY"));
    g_settings.volumeX  = clampNudge(Wh_GetIntSetting(L"volumeOffsetX"));
    g_settings.volumeY  = clampNudge(Wh_GetIntSetting(L"volumeOffsetY"));
    g_settings.batteryX = clampNudge(Wh_GetIntSetting(L"batteryOffsetX"));
    g_settings.batteryY = clampNudge(Wh_GetIntSetting(L"batteryOffsetY"));
    g_settings.percentX = clampNudge(Wh_GetIntSetting(L"percentOffsetX"));
    g_settings.percentY = clampNudge(Wh_GetIntSetting(L"percentOffsetY"));

    LoadColorSetting(L"wifiColor",      g_settings.wifiColor);
    LoadColorSetting(L"wifiColorTo",    g_settings.wifiColorTo);
    LoadColorSetting(L"volumeColor",    g_settings.volumeColor);
    LoadColorSetting(L"volumeColorTo",  g_settings.volumeColorTo);
    LoadColorSetting(L"batteryColor",   g_settings.batteryColor);
    LoadColorSetting(L"batteryColorTo", g_settings.batteryColorTo);
    LoadColorSetting(L"percentColor",   g_settings.percentColor);
    LoadColorSetting(L"percentColorTo", g_settings.percentColorTo);

    int dur = Wh_GetIntSetting(L"colorAnimateDuration");
    g_settings.colorAnimateDuration = (dur < 100 ? 2000 : dur > 30000 ? 30000 : dur);
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

static TextBlock g_wifiGlyphTB{ nullptr };
static TextBlock g_volumeGlyphTB{ nullptr };
static TextBlock g_batteryGlyphTB{ nullptr };
static TextBlock g_percentTB{ nullptr };

static std::vector<Storyboard> g_activeStoryboards;

static StackPanel         g_layoutUpdatedSP{ nullptr };
static winrt::event_token g_layoutUpdatedToken{};

static HWND g_taskbarWnd = nullptr;

static std::list<FrameworkElement::Loaded_revoker> g_autoRevokerList;

static bool CleanupLiveOmniButton();

// ── Grid geometry ─────────────────────────────────────────────────────────

struct GridGeom { int cols, rows, slotH; };

static GridGeom ResolveGeometry(int itemCount, HWND hTaskbarWnd) {
    int taskbarH = 48;
    if (hTaskbarWnd) {
        RECT r{}; if (GetWindowRect(hTaskbarWnd, &r)) taskbarH = r.bottom - r.top;
    }
    if (itemCount < 1) itemCount = 1;

    int rows = g_settings.gridRows;
    int cols = g_settings.gridColumns;

    if (rows <= 0 && cols <= 0) {
        // Full auto: prefer a single column whenever all items fit the taskbar
        // height (double-height taskbars — narrowest footprint, least empty
        // space); otherwise add columns (4 items on a single-height taskbar
        // become a 2x2). A 24px unit keeps auto rows readable; the slotHeight
        // setting overrides it.
        int unit = g_settings.slotHeight > 0 ? g_settings.slotHeight : 24;
        int rowsFit = std::max(1, taskbarH / std::max(1, unit));
        rows = std::min(itemCount, rowsFit);
        cols = (itemCount + rows - 1) / rows;
        rows = (itemCount + cols - 1) / cols;  // drop rows the items can't fill
    } else if (rows <= 0) {
        // Columns fixed by the user: rows follow from the item count.
        cols = std::min(cols, itemCount);
        rows = (itemCount + cols - 1) / cols;
    } else if (cols <= 0) {
        // Rows fixed by the user: columns follow from the item count.
        rows = std::min(rows, itemCount);
        cols = (itemCount + rows - 1) / rows;
    }
    if (rows < 1) rows = 1;
    if (cols < 1) cols = 1;

    int slotH = g_settings.slotHeight > 0 ? g_settings.slotHeight : (taskbarH / rows);
    if (slotH > 32) slotH = 32;
    if (slotH < 16) slotH = 16;
    return { cols, rows, slotH };
}

static void ItemGridPos(int idx, int cols, int rows, FillOrder fill,
                        ShortAlign align, int itemCount,
                        int& outCol, int& outRow)
{
    if (fill == FillOrder::ColumnFirst) {
        int fullCols = itemCount / rows, rem = itemCount % rows;
        outCol = idx / rows; outRow = idx % rows;
        if (rem > 0 && outCol == fullCols) {
            int gap = rows - rem;
            if (align == ShortAlign::Center) outRow += gap / 2;
            else if (align == ShortAlign::End) outRow += gap;
        }
    } else {
        int fullRows = itemCount / cols, rem = itemCount % cols;
        outRow = idx / cols; outCol = idx % cols;
        if (rem > 0 && outRow == fullRows) {
            int gap = cols - rem;
            if (align == ShortAlign::Center) outCol += gap / 2;
            else if (align == ShortAlign::End) outCol += gap;
        }
    }
}

static void ShortGroupCenterNudge(int gridPos, int cols, int rows, FillOrder fill,
                                  int itemCount, int slotW, int slotH,
                                  int& nudgeX, int& nudgeY) {
    nudgeX = nudgeY = 0;
    if (fill == FillOrder::RowFirst) {
        int rem = itemCount % cols;
        if (rem > 0 && (gridPos / cols) == (itemCount / cols)) {
            if ((cols - rem) % 2 == 1) nudgeX = slotW / 2;
        }
    } else {
        int rem = itemCount % rows;
        if (rem > 0 && (gridPos / rows) == (itemCount / rows)) {
            if ((rows - rem) % 2 == 1) nudgeY = slotH / 2;
        }
    }
}

// ── XAML helpers ──────────────────────────────────────────────────────────

static void ApplyOffset(FrameworkElement const& fe, int x, int y) {
    if (x != 0 || y != 0) {
        TranslateTransform tt; tt.X(double(x)); tt.Y(double(y));
        fe.RenderTransform(tt);
    } else {
        fe.ClearValue(UIElement::RenderTransformProperty());
    }
}

static void ClearLayoutProperties(FrameworkElement const& fe) {
    if (!fe) return;
    fe.ClearValue(FrameworkElement::WidthProperty());
    fe.ClearValue(FrameworkElement::MinWidthProperty());
    fe.ClearValue(FrameworkElement::MaxWidthProperty());
    fe.ClearValue(FrameworkElement::HeightProperty());
    fe.ClearValue(FrameworkElement::MinHeightProperty());
    fe.ClearValue(FrameworkElement::MaxHeightProperty());
    fe.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
    fe.ClearValue(FrameworkElement::VerticalAlignmentProperty());
    fe.ClearValue(FrameworkElement::MarginProperty());
    fe.ClearValue(UIElement::RenderTransformProperty());
}

// Clear RenderTransform on every FrameworkElement in a subtree. Cleanup
// safety net: if Windows re-templated an element between apply and cleanup
// (observed with the battery percent TextBlock), the cached ref no longer
// matches the live element and a stale TranslateTransform survives disable.
static void ClearTransformsRecursive(DependencyObject const& node, int depth = 0) {
    if (depth > 6) return;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        if (auto fe = child.try_as<FrameworkElement>()) {
            try { fe.ClearValue(UIElement::RenderTransformProperty()); } catch (...) {}
        }
        ClearTransformsRecursive(child, depth + 1);
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

static bool WalkSetupBatteryInnerPanel(DependencyObject const& node, int slotW, int depth = 0) {
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
                auto glyph = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
                if (glyph) {
                    g_batteryGlyphFE = glyph;
                    glyph.Width(double(slotW));
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
                Wh_Log(L"[Battery] inner SP has %d children — no %% element", spN);
            }
            return true;
        }
        if (WalkSetupBatteryInnerPanel(child, slotW, depth + 1)) return true;
    }
    return false;
}

// ── Color / animation helpers ─────────────────────────────────────────────

// Color-returning variant of the canonical token parser (_templates/button-surface.h):
// "#RRGGBB" / "#AARRGGBB" hex (bare hex also accepted for back-compat), the
// generics "accent" / "accentLight" / "accentDark" / "transparent", and the
// numbered Windows shades "accentLight1"-"3" / "accentDark1"-"3" (accepted
// silently, undocumented). Empty/unparseable returns false = keep native.
static bool ParseHexColor(const wchar_t* s, Color& out) {
    using winrt::Windows::UI::ViewManagement::UIColorType;
    if (!s || !*s) return false;

    if (_wcsicmp(s, L"transparent") == 0) {
        out = {0, 0, 0, 0};
        return true;
    }

    static const struct { const wchar_t* token; UIColorType type; } kAccentTokens[] = {
        {L"accent",       UIColorType::Accent},
        {L"accentLight",  UIColorType::AccentLight2},
        {L"accentDark",   UIColorType::AccentDark1},
        {L"accentLight1", UIColorType::AccentLight1},
        {L"accentLight2", UIColorType::AccentLight2},
        {L"accentLight3", UIColorType::AccentLight3},
        {L"accentDark1",  UIColorType::AccentDark1},
        {L"accentDark2",  UIColorType::AccentDark2},
        {L"accentDark3",  UIColorType::AccentDark3},
    };
    for (auto const& entry : kAccentTokens) {
        if (_wcsicmp(s, entry.token) == 0) {
            try {
                winrt::Windows::UI::ViewManagement::UISettings settings;
                out = settings.GetColorValue(entry.type);
                return true;
            } catch (...) {
                Wh_Log(L"[Color] Failed to read the Windows accent color");
                return false;
            }
        }
    }

    const wchar_t* p = (*s == L'#') ? s + 1 : s;
    size_t len = wcslen(p);
    if (len != 6 && len != 8) return false;
    for (size_t i = 0; i < len; i++)
        if (!iswxdigit(p[i])) return false;
    wchar_t buf[9]{};
    wcsncpy(buf, p, 8);
    unsigned long v = wcstoul(buf, nullptr, 16);
    if (len == 6) { out = { 255, BYTE(v>>16), BYTE(v>>8), BYTE(v) }; }
    else          { out = { BYTE(v>>24), BYTE(v>>16), BYTE(v>>8), BYTE(v) }; }
    return true;
}

// Walk subtree looking for TextBlock named "InnerTextBlock" (the standard glyph element
// in Windows 11 system tray icon templates). Returns nullptr if not found.
static TextBlock FindInnerTextBlock(DependencyObject const& root, int depth = 0) {
    if (depth > 12) return nullptr;
    auto fe = root.try_as<FrameworkElement>();
    if (fe && fe.Name() == L"InnerTextBlock") {
        if (auto tb = fe.try_as<TextBlock>()) return tb;
    }
    int n = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        if (auto found = FindInnerTextBlock(child, depth + 1)) return found;
    }
    return nullptr;
}

static TextBlock FindFirstTextBlock(DependencyObject const& root, int depth = 0) {
    if (depth > 12) return nullptr;
    if (auto tb = root.try_as<TextBlock>()) return tb;
    int n = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(root, i);
        if (!child) continue;
        if (auto found = FindFirstTextBlock(child, depth + 1)) return found;
    }
    return nullptr;
}

static TextBlock AcquireGlyphTB(FrameworkElement const& root) {
    if (!root) return nullptr;
    if (auto tb = root.try_as<TextBlock>()) return tb;
    if (auto tb = FindInnerTextBlock(root)) return tb;
    return FindFirstTextBlock(root);  // fallback for battery/non-standard templates
}

static void StopAndClearStoryboards() {
    for (auto& sb : g_activeStoryboards) { try { sb.Stop(); } catch (...) {} }
    g_activeStoryboards.clear();
}

static void ClearGlyphColors() {
    StopAndClearStoryboards();
    auto clearTB = [](TextBlock const& tb) {
        if (tb) try { tb.ClearValue(TextBlock::ForegroundProperty()); } catch (...) {}
    };
    clearTB(g_wifiGlyphTB);
    clearTB(g_volumeGlyphTB);
    clearTB(g_batteryGlyphTB);
    clearTB(g_percentTB);
}

static void ApplyGlyphColor(TextBlock const& tb,
                            const wchar_t* fromHex, const wchar_t* toHex, int durMs) {
    if (!tb) return;
    Color fromColor{}, toColor{};
    bool hasFrom = ParseHexColor(fromHex, fromColor);
    bool hasTo   = ParseHexColor(toHex,   toColor);
    if (!hasFrom) {
        try { tb.ClearValue(TextBlock::ForegroundProperty()); } catch (...) {}
        return;
    }
    SolidColorBrush brush;
    brush.Color(fromColor);
    try { tb.Foreground(brush); } catch (...) { return; }
    if (hasTo) {
        try {
            ColorAnimation anim;
            anim.From(fromColor);
            anim.To(toColor);
            TimeSpan ts{ int64_t(durMs) * 10000LL };
            anim.Duration(DurationHelper::FromTimeSpan(ts));
            anim.AutoReverse(true);
            anim.RepeatBehavior(RepeatBehaviorHelper::Forever());
            Storyboard sb;
            sb.Children().Append(anim);
            Storyboard::SetTarget(anim, brush);
            Storyboard::SetTargetProperty(anim, L"Color");
            sb.Begin();
            g_activeStoryboards.push_back(sb);
        } catch (...) {}
    }
}

static void ApplyAllColors() {
    if (!g_wifiGlyphTB   && g_wifiPresenter)    g_wifiGlyphTB   = AcquireGlyphTB(g_wifiPresenter);
    if (!g_volumeGlyphTB && g_volumePresenter)   g_volumeGlyphTB = AcquireGlyphTB(g_volumePresenter);
    if (!g_batteryGlyphTB && g_batteryGlyphFE)   g_batteryGlyphTB = AcquireGlyphTB(g_batteryGlyphFE);
    if (!g_percentTB && g_batteryPercentFE)       g_percentTB = g_batteryPercentFE.try_as<TextBlock>();

    StopAndClearStoryboards();
    ApplyGlyphColor(g_wifiGlyphTB,    g_settings.wifiColor,    g_settings.wifiColorTo,    g_settings.colorAnimateDuration);
    ApplyGlyphColor(g_volumeGlyphTB,  g_settings.volumeColor,  g_settings.volumeColorTo,  g_settings.colorAnimateDuration);
    ApplyGlyphColor(g_batteryGlyphTB, g_settings.batteryColor, g_settings.batteryColorTo, g_settings.colorAnimateDuration);
    ApplyGlyphColor(g_percentTB,      g_settings.percentColor, g_settings.percentColorTo, g_settings.colorAnimateDuration);
}

// ── OmniButton chrome / internal footprint ────────────────────────────────

static void ApplyOmniButtonChrome() {
    if (!g_omniButton) return;
    double pad = double(g_settings.buttonHorizontalPadding);
    auto ctrl = g_omniButton.try_as<Control>();
    if (ctrl) {
        ctrl.Padding(Thickness{ pad, 0.0, pad, 0.0 });
        ctrl.HorizontalContentAlignment(HorizontalAlignment::Center);
        ctrl.VerticalContentAlignment(VerticalAlignment::Center);
    }
    g_omniButton.ClearValue(FrameworkElement::WidthProperty());
    g_omniButton.ClearValue(FrameworkElement::MinWidthProperty());
    g_omniButton.ClearValue(FrameworkElement::MaxWidthProperty());
    g_omniButton.ClearValue(FrameworkElement::HeightProperty());
    g_omniButton.ClearValue(FrameworkElement::MinHeightProperty());
    g_omniButton.ClearValue(FrameworkElement::MaxHeightProperty());
    g_omniButton.ClearValue(FrameworkElement::HorizontalAlignmentProperty());
    g_omniButton.InvalidateMeasure();
}

static void ApplyItemsHostFootprint(StackPanel const& sp, const GridGeom& geom) {
    sp.Width(double(geom.cols * g_settings.slotWidth));
    sp.Height(double(geom.rows * geom.slotH));
    sp.HorizontalAlignment(HorizontalAlignment::Center);
    sp.VerticalAlignment(VerticalAlignment::Center);
    sp.InvalidateMeasure();
}

// ── XAML cleanup ──────────────────────────────────────────────────────────

static void CleanupXamlElements(
    StackPanel sp, FrameworkElement btn,
    FrameworkElement wifi, FrameworkElement vol,
    FrameworkElement bp, StackPanel bip,
    FrameworkElement biglyph, FrameworkElement bipct)
{
    // Stop storyboards first so animations don't race with element cleanup.
    // Foreground clearing is handled by CleanupAndResetCurrentElements while refs are still live.
    StopAndClearStoryboards();

    try {
        if (sp) {
            sp.Orientation(Orientation::Horizontal);
            sp.ClearValue(StackPanel::SpacingProperty());
            ClearLayoutProperties(sp);
            int n = VisualTreeHelper::GetChildrenCount(sp);
            for (int i = 0; i < n; i++) {
                auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
                if (child) {
                    ClearLayoutProperties(child);
                    if (auto cp = child.try_as<ContentPresenter>()) {
                        cp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                        cp.ClearValue(ContentPresenter::VerticalContentAlignmentProperty());
                    }
                }
            }
        }
    } catch (...) {}
    try {
        if (btn) {
            ClearLayoutProperties(btn);
            if (auto ctrl = btn.try_as<Control>()) {
                ctrl.ClearValue(Control::PaddingProperty());
                ctrl.ClearValue(Control::HorizontalContentAlignmentProperty());
                ctrl.ClearValue(Control::VerticalContentAlignmentProperty());
            }
        }
    } catch (...) {}
    try { if (wifi) wifi.ClearValue(UIElement::RenderTransformProperty()); } catch (...) {}
    try { if (vol)  vol.ClearValue(UIElement::RenderTransformProperty());  } catch (...) {}
    try {
        if (bp) {
            ClearLayoutProperties(bp);
            if (auto bcp = bp.try_as<ContentPresenter>()) {
                bcp.ClearValue(ContentPresenter::HorizontalContentAlignmentProperty());
                bcp.ClearValue(ContentPresenter::VerticalContentAlignmentProperty());
            }
            // Sweep the whole battery subtree for stale transforms — the
            // cached glyph/percent refs can go stale if Windows re-templated
            // the battery content (stranded "80%" after disable).
            ClearTransformsRecursive(bp);
        }
    } catch (...) {}
    try {
        if (bip) {
            bip.Orientation(Orientation::Horizontal); // template default is Vertical; must set explicitly
            bip.ClearValue(StackPanel::SpacingProperty());
            int n = VisualTreeHelper::GetChildrenCount(bip);
            for (int i = 0; i < n; i++) {
                auto fe = VisualTreeHelper::GetChild(bip, i).try_as<FrameworkElement>();
                if (fe) {
                    ClearLayoutProperties(fe);
                    if (auto tb = fe.try_as<TextBlock>()) tb.ClearValue(TextBlock::TextAlignmentProperty());
                }
            }
        }
    } catch (...) {}
    try { if (biglyph) ClearLayoutProperties(biglyph); } catch (...) {}
    try {
        if (bipct) {
            ClearLayoutProperties(bipct);
            if (auto tb = bipct.try_as<TextBlock>()) tb.ClearValue(TextBlock::TextAlignmentProperty());
        }
    } catch (...) {}
    try {
        if (btn) {
            btn.InvalidateMeasure(); btn.InvalidateArrange();
            if (auto p = VisualTreeHelper::GetParent(btn).try_as<UIElement>()) {
                p.InvalidateMeasure(); p.InvalidateArrange();
                if (auto gp = VisualTreeHelper::GetParent(p).try_as<UIElement>()) {
                    gp.InvalidateMeasure(); gp.InvalidateArrange();
                }
            }
            btn.UpdateLayout();
        }
    } catch (...) {}
}

static void ResetElementRefs() {
    g_omniStackPanel = nullptr; g_omniButton = nullptr;
    g_wifiPresenter = nullptr;  g_volumePresenter = nullptr;
    g_batteryPresenter = nullptr; g_batteryInnerPanel = nullptr;
    g_batteryGlyphFE = nullptr; g_batteryPercentFE = nullptr;
    g_wifiGlyphTB = nullptr;    g_volumeGlyphTB = nullptr;
    g_batteryGlyphTB = nullptr; g_percentTB = nullptr;
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
    // Clear glyph colors (stops storyboards, clears Foreground) while refs are still live
    ClearGlyphColors();
    auto sp      = g_omniStackPanel; auto btn     = g_omniButton;
    auto wifi    = g_wifiPresenter;  auto vol     = g_volumePresenter;
    auto bp      = g_batteryPresenter;
    auto bip     = g_batteryInnerPanel;
    auto biglyph = g_batteryGlyphFE;  auto bipct   = g_batteryPercentFE;
    ResetElementRefs();
    CleanupXamlElements(sp, btn, wifi, vol, bp, bip, biglyph, bipct);
    CleanupLiveOmniButton();
}

// ── Layout application ────────────────────────────────────────────────────

static void ResolveItemPositions(const wchar_t* orderStr, bool hasBattery, bool hasPercent,
                                  int posMap[4])
{
    const bool present[4] = { true, true, hasBattery, hasPercent };
    const wchar_t* const names[4] = { L"wifi", L"volume", L"battery", L"percent" };
    bool assigned[4] = {};
    int nextPos = 0;
    const wchar_t* p = orderStr;
    while (*p) {
        while (*p == L' ' || *p == L'\t' || *p == L',') p++;
        if (!*p) break;
        const wchar_t* start = p;
        while (*p && *p != L' ' && *p != L'\t' && *p != L',') p++;
        size_t len = size_t(p - start);
        for (int i = 0; i < 4; i++) {
            if (!assigned[i] && present[i] &&
                wcslen(names[i]) == len && wcsncmp(start, names[i], len) == 0)
            { posMap[i] = nextPos++; assigned[i] = true; break; }
        }
    }
    for (int i = 0; i < 4; i++) if (present[i] && !assigned[i]) posMap[i] = nextPos++;
    for (int i = 0; i < 4; i++) if (!present[i]) posMap[i] = -1;
}

static void PositionSlot(FrameworkElement const& fe, int slotIdx,
                         int col, int row, const GridGeom& geom,
                         int nudgeX, int nudgeY)
{
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

    // Locate battery slot by class-name substring search
    int battIdx = -1;
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
        if (child && HasBatteryDescendant(child)) { battIdx = i; break; }
    }

    bool hasBattPres = battIdx >= 0;
    bool hasPercent  = false;
    if (hasBattPres) {
        g_batteryPresenter = VisualTreeHelper::GetChild(sp, battIdx).try_as<FrameworkElement>();
        if (WalkSetupBatteryInnerPanel(g_batteryPresenter, g_settings.slotWidth))
            hasPercent = (g_batteryPercentFE != nullptr);
    }

    // Log any unknown slots (future Windows builds may add more items)
    for (int i = 0; i < n; i++) {
        if (i == 0 || i == 1 || i == battIdx) continue;
        auto child = VisualTreeHelper::GetChild(sp, i);
        if (child) Wh_Log(L"[Layout] Unknown slot index %d: %s", i, winrt::get_class_name(child).c_str());
    }

    int itemCount = hasBattPres ? (hasPercent ? 4 : 3) : 2;
    GridGeom geom = ResolveGeometry(itemCount, hTaskbarWnd);

    int posMap[4];
    ResolveItemPositions(g_settings.itemOrderStr, hasBattPres, hasPercent, posMap);
    Wh_Log(L"[Layout] items=%d cols=%d rows=%d slotH=%d order=[%d,%d,%d,%d] mode=%s",
        itemCount, geom.cols, geom.rows, geom.slotH,
        posMap[0], posMap[1], posMap[2], posMap[3],
        g_settings.batteryPercentMode == BattPctMode::Independent ? L"indep" : L"coupled");

    if (g_omniButton) ApplyOmniButtonChrome();
    ApplyItemsHostFootprint(sp, geom);

    // Helper: resolve (col, row) + short-group center nudge for a grid position
    auto resolvePos = [&](int gridPos, int& col, int& row, int& cnx, int& cny) {
        ItemGridPos(gridPos, geom.cols, geom.rows,
                    g_settings.fillOrder, g_settings.shortGroupAlign, itemCount, col, row);
        cnx = cny = 0;
        if (g_settings.shortGroupAlign == ShortAlign::Center)
            ShortGroupCenterNudge(gridPos, geom.cols, geom.rows, g_settings.fillOrder,
                                  itemCount, g_settings.slotWidth, geom.slotH, cnx, cny);
    };

    // ── Wifi (slot 0) ──
    if (n >= 1) {
        auto wifi = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
        if (wifi) {
            g_wifiPresenter = wifi;
            wifi.Width(double(g_settings.slotWidth)); wifi.Height(double(geom.slotH));
            wifi.HorizontalAlignment(HorizontalAlignment::Left);
            if (auto cp = wifi.try_as<ContentPresenter>()) {
                cp.HorizontalContentAlignment(HorizontalAlignment::Center);
                cp.VerticalContentAlignment(VerticalAlignment::Center);
            }
            int col, row, cnx, cny; resolvePos(posMap[0], col, row, cnx, cny);
            PositionSlot(wifi, 0, col, row, geom, g_settings.wifiX + cnx, g_settings.wifiY + cny);
        }
    }
    // ── Volume (slot 1) ──
    if (n >= 2) {
        auto vol = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
        if (vol) {
            g_volumePresenter = vol;
            vol.Width(double(g_settings.slotWidth)); vol.Height(double(geom.slotH));
            vol.HorizontalAlignment(HorizontalAlignment::Left);
            if (auto cp = vol.try_as<ContentPresenter>()) {
                cp.HorizontalContentAlignment(HorizontalAlignment::Center);
                cp.VerticalContentAlignment(VerticalAlignment::Center);
            }
            int col, row, cnx, cny; resolvePos(posMap[1], col, row, cnx, cny);
            PositionSlot(vol, 1, col, row, geom, g_settings.volumeX + cnx, g_settings.volumeY + cny);
        }
    }

    // ── Battery + percent ──
    if (hasBattPres && g_batteryPresenter) {
        if (g_settings.batteryPercentMode == BattPctMode::Independent && hasPercent) {
            // Independent mode: battery CP spans the full grid, positioned at the grid origin.
            // Inner SP is left at its natural size; glyph and percent are offset absolutely
            // within the CP's coordinate space via RenderTransform.
            //
            //   battery CP transform.Y  = -(battIdx * slotH)   [moves CP to grid top]
            //   glyph    transform      = (bCol*slotW,  bRow*slotH) + nudge
            //   percent  transform      = ((pCol-1)*slotW, pRow*slotH) + nudge
            //     [percent natural X = slotW (glyph width); (pCol-1)*slotW corrects for it]

            g_batteryPresenter.Width(double(geom.cols * g_settings.slotWidth));
            g_batteryPresenter.Height(double(geom.rows * geom.slotH));
            g_batteryPresenter.HorizontalAlignment(HorizontalAlignment::Left);
            g_batteryPresenter.VerticalAlignment(VerticalAlignment::Top);
            if (auto bcp = g_batteryPresenter.try_as<ContentPresenter>()) {
                bcp.HorizontalContentAlignment(HorizontalAlignment::Left);
                bcp.VerticalContentAlignment(VerticalAlignment::Top);
            }
            // Move CP to grid origin (cancel out battIdx rows of natural stacking)
            ApplyOffset(g_batteryPresenter, 0, -(battIdx * geom.slotH));

            // Override inner SP children to Top-aligned so Y is predictable
            if (g_batteryGlyphFE) g_batteryGlyphFE.VerticalAlignment(VerticalAlignment::Top);
            if (g_batteryPercentFE) g_batteryPercentFE.VerticalAlignment(VerticalAlignment::Top);

            // Absolute position of battery glyph within battery CP
            int bCol, bRow, bcnx, bcny; resolvePos(posMap[2], bCol, bRow, bcnx, bcny);
            if (g_batteryGlyphFE)
                ApplyOffset(g_batteryGlyphFE,
                    bCol * g_settings.slotWidth + g_settings.batteryX + bcnx,
                    bRow * geom.slotH           + g_settings.batteryY + bcny);

            // Absolute position of percent within battery CP
            int pCol, pRow, pcnx, pcny; resolvePos(posMap[3], pCol, pRow, pcnx, pcny);
            if (g_batteryPercentFE)
                ApplyOffset(g_batteryPercentFE,
                    (pCol - 1) * g_settings.slotWidth + g_settings.percentX + pcnx,
                    pRow * geom.slotH                 + g_settings.percentY + pcny);

            Wh_Log(L"[Layout] Indep batt=grid(%d,%d) pct=grid(%d,%d)", bCol, bRow, pCol, pRow);
        } else {
            // Coupled mode: battery CP at its grid cell, percent offset relative to battery
            double battW = double(g_settings.slotWidth * (hasPercent ? 2 : 1));
            g_batteryPresenter.Width(battW);
            g_batteryPresenter.Height(double(geom.slotH));
            g_batteryPresenter.HorizontalAlignment(HorizontalAlignment::Left);
            if (auto bcp = g_batteryPresenter.try_as<ContentPresenter>()) {
                // Left-pin the inner panel so the percent's natural X is exactly
                // one glyph-slot from the cell origin — the relX math below
                // assumes it; native centering would skew it by half the slack.
                bcp.HorizontalContentAlignment(HorizontalAlignment::Left);
                bcp.VerticalContentAlignment(VerticalAlignment::Center);
            }
            int bCol, bRow, bcnx, bcny; resolvePos(posMap[2], bCol, bRow, bcnx, bcny);
            PositionSlot(g_batteryPresenter, battIdx, bCol, bRow, geom,
                         g_settings.batteryX + bcnx, g_settings.batteryY + bcny);

            if (hasPercent && g_batteryPercentFE) {
                int pCol, pRow, pcnx, pcny; resolvePos(posMap[3], pCol, pRow, pcnx, pcny);
                int relX = (pCol - bCol - 1) * g_settings.slotWidth
                         + g_settings.percentX + (pcnx - bcnx);
                int relY = (pRow - bRow) * geom.slotH
                         + g_settings.percentY + (pcny - bcny);
                ApplyOffset(g_batteryPercentFE, relX, relY);
                Wh_Log(L"[Layout] Coupled pct at grid(%d,%d) batt at grid(%d,%d) relX=%d relY=%d",
                    pCol, pRow, bCol, bRow, relX, relY);
            }
        }
    }

    Wh_Log(L"[Layout] Applied grid (SP children=%d)", n);

    // Best-effort color application; also retried in OnLayoutUpdated
    ApplyAllColors();
}

// ── Deferred layout (LayoutUpdated) ──────────────────────────────────────

static void OnLayoutUpdated(IInspectable const&, IInspectable const&) {
    auto sp = g_layoutUpdatedSP;
    if (!sp) return;

    bool allReady = g_wifiPresenter && g_volumePresenter &&
                    (!g_batteryPresenter ||
                     (g_batteryInnerPanel && g_batteryPercentFE));
    if (allReady) {
        sp.LayoutUpdated(g_layoutUpdatedToken); g_layoutUpdatedToken = {};
        g_layoutUpdatedSP = nullptr;
        ApplyAllColors();
        return;
    }

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
    // Percent TextBlock materialized after the initial layout (e.g. the user
    // flipped the Windows battery-percent switch while the mod was applied).
    if (!percentNowPresent && g_batteryInnerPanel && !g_batteryPercentFE &&
        VisualTreeHelper::GetChildrenCount(g_batteryInnerPanel) >= 2) {
        percentNowPresent = true;
    }

    if (!batteryNowPresent && !percentNowPresent) return;

    Wh_Log(L"[Layout] Deferred: battery=%d percent=%d — re-applying",
        (int)batteryNowPresent, (int)percentNowPresent);

    sp.LayoutUpdated(g_layoutUpdatedToken); g_layoutUpdatedToken = {};
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
        DWORD pid; WCHAR cls[32];
        if (GetWindowThreadProcessId(hWnd, &pid) && pid == GetCurrentProcessId() &&
            GetClassName(hWnd, cls, ARRAYSIZE(cls)) && _wcsicmp(cls, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(lParam) = hWnd; return FALSE;
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
            auto* cwp = (const CWPSTRUCT*)lParam;
            if (cwp->message == kMsg) { auto* p = (Param*)cwp->lParam; p->proc(p->procParam); }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }, nullptr, dwThreadId);
    if (!hook) return false;
    Param param{ proc, procParam };
    SendMessage(hWnd, kMsg, 0, (LPARAM)&param);
    UnhookWindowsHookEx(hook);
    return true;
}

// ── GetTaskbarXamlRoot ────────────────────────────────────────────────────

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void*, void*);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;
using TaskbarHost_FrameHeight_t  = int (WINAPI*)(void*);
TaskbarHost_FrameHeight_t  TaskbarHost_FrameHeight_Original;
using std__Ref_count_base__Decref_t = void (WINAPI*)(void*);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;
static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    if (!CTaskBand_GetTaskbarHost_Original || !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable) return nullptr;
    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;
    void* taskBand = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    if (!taskBand) return nullptr;
    void* tbfs = taskBand;
    for (int i = 0; *(void**)tbfs != CTaskBand_ITaskListWndSite_vftable; i++) {
        if (i == 20) return nullptr; tbfs = (void**)tbfs + 1;
    }
    void* hsp[2]{};
    CTaskBand_GetTaskbarHost_Original(tbfs, hsp);
    if (!hsp[0] || !hsp[1]) {
        if (hsp[1]) std__Ref_count_base__Decref_Original(hsp[1]);
        return nullptr;
    }
    size_t offset = 0x10;
#if defined(_M_X64)
    {
        const BYTE* b = (const BYTE*)TaskbarHost_FrameHeight_Original;
        if (b[0]==0x48 && b[1]==0x83 && b[2]==0xEC && b[4]==0x48 &&
            b[5]==0x83 && b[6]==0xC1 && b[7]<=0x7F)
            offset = b[7];
        else Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    {
        const DWORD* p = (const DWORD*)TaskbarHost_FrameHeight_Original;
        if (p[0]==0xD503237F && (p[1]&0xFFC07FFF)==0xA9807BFD &&
            p[2]==0x910003FD && (p[3]&0xFFF00FE0)==0xF8400C00)
            offset = (p[3] >> 12) & 0xFF;
        else Wh_Log(L"Unsupported TaskbarHost::FrameHeight");
    }
#else
#error "Unsupported architecture"
#endif
    auto* iunk = *(IUnknown**)((BYTE*)hsp[0] + offset);
    if (!iunk) { std__Ref_count_base__Decref_Original(hsp[1]); return nullptr; }
    FrameworkElement taskbarElem = nullptr;
    iunk->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(taskbarElem));
    auto result = taskbarElem ? taskbarElem.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(hsp[1]);
    return result;
}

// ── XAML tree helpers ─────────────────────────────────────────────────────

static FrameworkElement FindChildByClassName(FrameworkElement const& e, PCWSTR cls) {
    int n = VisualTreeHelper::GetChildrenCount(e);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(e, i).try_as<FrameworkElement>();
        if (child && winrt::get_class_name(child) == cls) return child;
    }
    return nullptr;
}
static FrameworkElement FindChildByName(FrameworkElement const& e, PCWSTR name) {
    int n = VisualTreeHelper::GetChildrenCount(e);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(e, i).try_as<FrameworkElement>();
        if (child && child.Name() == name) return child;
    }
    return nullptr;
}
static FrameworkElement FindChildRecursive(FrameworkElement const& e,
    std::function<bool(FrameworkElement)> const& cb, int maxDepth = 20)
{
    int n = VisualTreeHelper::GetChildrenCount(e);
    for (int i = 0; i < n && maxDepth > 0; i++) {
        auto child = VisualTreeHelper::GetChild(e, i).try_as<FrameworkElement>();
        if (!child) continue;
        if (cb(child)) return child;
        auto found = FindChildRecursive(child, cb, maxDepth - 1);
        if (found) return found;
    }
    return nullptr;
}

static bool FindBatteryInnerElements(DependencyObject const& node,
                                     StackPanel& ip, FrameworkElement& glyph,
                                     FrameworkElement& pct, int depth = 0) {
    if (depth > 8) return false;
    int n = VisualTreeHelper::GetChildrenCount(node);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(node, i);
        if (!child) continue;
        auto sp = child.try_as<StackPanel>();
        if (sp && !sp.IsItemsHost()) {
            ip = sp;
            int spN = VisualTreeHelper::GetChildrenCount(sp);
            if (spN >= 1) glyph = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
            if (spN >= 2) pct   = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
            return true;
        }
        if (FindBatteryInnerElements(child, ip, glyph, pct, depth + 1)) return true;
    }
    return false;
}

static bool CleanupLiveOmniButton() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return false;
    try {
        auto xamlRoot = GetTaskbarXamlRoot(hWnd);
        if (!xamlRoot) return false;
        auto content = xamlRoot.Content().try_as<FrameworkElement>();
        if (!content) return false;
        auto omniBtn = FindChildRecursive(content, [](FrameworkElement fe) {
            return fe.Name() == L"ControlCenterButton"; });
        if (!omniBtn) return false;
        auto grid = FindChildByClassName(omniBtn, L"Windows.UI.Xaml.Controls.Grid");
        auto cp   = grid ? FindChildByName(grid, L"ContentPresenter") : nullptr;
        auto ip   = cp   ? FindChildByClassName(cp, L"Windows.UI.Xaml.Controls.ItemsPresenter") : nullptr;
        auto sp   = ip   ? FindChildByClassName(ip, L"Windows.UI.Xaml.Controls.StackPanel").try_as<StackPanel>() : nullptr;
        if (!sp || !sp.IsItemsHost()) return false;
        FrameworkElement wifi{nullptr}, vol{nullptr}, batt{nullptr};
        FrameworkElement bglyph{nullptr}, bpct{nullptr};
        StackPanel bip{nullptr};
        int nc = VisualTreeHelper::GetChildrenCount(sp);
        if (nc >= 1) wifi = VisualTreeHelper::GetChild(sp, 0).try_as<FrameworkElement>();
        if (nc >= 2) vol  = VisualTreeHelper::GetChild(sp, 1).try_as<FrameworkElement>();
        for (int i = 0; i < nc; i++) {
            auto child = VisualTreeHelper::GetChild(sp, i).try_as<FrameworkElement>();
            if (child && HasBatteryDescendant(child)) {
                batt = child;
                FindBatteryInnerElements(child, bip, bglyph, bpct);
                break;
            }
        }
        CleanupXamlElements(sp, omniBtn, wifi, vol, batt, bip, bglyph, bpct);
        Wh_Log(L"[Cleanup] Live OmniButton cleanup applied");
        return true;
    } catch (...) {
        Wh_Log(L"[Cleanup] Live OmniButton cleanup failed");
        return false;
    }
}

// ── Apply settings ────────────────────────────────────────────────────────

static void ApplyAllSettings() {
    HWND hWnd = FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return; }
    g_taskbarWnd = hWnd;
    try {
        auto xamlRoot = GetTaskbarXamlRoot(hWnd);
        if (!xamlRoot) { Wh_Log(L"[Apply] GetTaskbarXamlRoot failed"); return; }
        auto content = xamlRoot.Content().try_as<FrameworkElement>();
        if (!content) return;
        if (!g_omniStackPanel) {
            auto omniBtn = FindChildRecursive(content, [](FrameworkElement fe) {
                return fe.Name() == L"ControlCenterButton"; });
            if (omniBtn) {
                g_omniButton = omniBtn;
                auto grid = FindChildByClassName(omniBtn, L"Windows.UI.Xaml.Controls.Grid");
                auto cp   = grid ? FindChildByName(grid, L"ContentPresenter") : nullptr;
                auto ip   = cp   ? FindChildByClassName(cp, L"Windows.UI.Xaml.Controls.ItemsPresenter") : nullptr;
                auto sp   = ip   ? FindChildByClassName(ip, L"Windows.UI.Xaml.Controls.StackPanel").try_as<StackPanel>() : nullptr;
                if (sp && sp.IsItemsHost()) {
                    ApplyLayout(sp, hWnd);
                    bool needsDeferred = !g_wifiPresenter || !g_volumePresenter ||
                                         !g_batteryPresenter ||
                                         (g_batteryPresenter &&
                                          (!g_batteryInnerPanel || !g_batteryPercentFE));
                    if (needsDeferred) {
                        g_layoutUpdatedSP = sp;
                        g_layoutUpdatedToken = sp.LayoutUpdated(OnLayoutUpdated);
                        Wh_Log(L"[Apply] Registered LayoutUpdated for deferred elements");
                    }
                } else Wh_Log(L"[Apply] IsItemsHost StackPanel not found");
            } else Wh_Log(L"[Apply] ControlCenterButton not found");
        } else {
            // Already applied layout; re-apply colors only (e.g. after settings change path)
            ApplyAllColors();
        }
    } catch (...) { Wh_Log(L"[Apply] Exception (XAML not ready)"); }
}

static void ApplyAllSettingsOnWindowThread() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) { ApplyAllSettings(); }, nullptr);
}

// ── IconView constructor hook ──────────────────────────────────────────────

using IconView_IconView_t = void*(WINAPI*)(void*);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* ret = IconView_IconView_Original(pThis);
    FrameworkElement iconView = nullptr;
    ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(), winrt::put_abi(iconView));
    if (!iconView) return ret;
    g_autoRevokerList.emplace_back();
    auto it = std::prev(g_autoRevokerList.end());
    *it = iconView.Loaded(winrt::auto_revoke_t{},
        [it](IInspectable const&, RoutedEventArgs const&) {
            g_autoRevokerList.erase(it);
            if (!g_unloading && !g_omniStackPanel) ApplyAllSettings();
        });
    return ret;
}

// ── System tray module detection and hook setup ───────────────────────────

using LoadLibraryExW_t = decltype(&LoadLibraryExW);
LoadLibraryExW_t LoadLibraryExW_Original;
static std::atomic<bool> g_systemTrayModuleHooked = false;

static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE hModule, UINT* puPtrLen) {
    void* pInfo = nullptr; UINT uLen = 0;
    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
    if (hRes) {
        HGLOBAL hGlob = LoadResource(hModule, hRes);
        if (hGlob) {
            void* pData = LockResource(hGlob);
            if (pData && !VerQueryValue(pData, L"\\", &pInfo, &uLen)) { pInfo = nullptr; uLen = 0; }
        }
    }
    if (puPtrLen) *puPtrLen = uLen;
    return (VS_FIXEDFILEINFO*)pInfo;
}

static HMODULE GetSystemTrayModuleHandle() {
    HMODULE m = GetModuleHandleW(L"SystemTray.dll");
    if (!m) {
        m = GetModuleHandleW(L"Taskbar.View.dll");
        if (m) {
            auto* fi = GetModuleVersionInfo(m, nullptr);
            WORD major = fi ? HIWORD(fi->dwFileVersionMS) : 0;
            if (!major || major >= 2604) { Wh_Log(L"[Hooks] Skipping Taskbar.View.dll v%d", major); m = nullptr; }
        }
    }
    if (!m) m = GetModuleHandleW(L"ExplorerExtensions.dll");
    return m;
}

static bool HookSystemTraySymbols(HMODULE hModule) {
    WindhawkUtils::SYMBOL_HOOK hooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original, IconView_IconView_Hook,
    }};
    if (!WindhawkUtils::HookSymbols(hModule, hooks, ARRAYSIZE(hooks))) {
        Wh_Log(L"[Hooks] HookSymbols failed"); return false;
    }
    return true;
}

static void HandleLoadedModuleIfSystemTray(HMODULE hModule, LPCWSTR lpLibFileName) {
    if (!g_systemTrayModuleHooked && GetSystemTrayModuleHandle() == hModule &&
        !g_systemTrayModuleHooked.exchange(true)) {
        Wh_Log(L"[LoadLib] %s — hooking symbols", lpLibFileName);
        if (HookSystemTraySymbols(hModule)) Wh_ApplyHookOperations();
    }
}

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (hModule && lpLibFileName) HandleLoadedModuleIfSystemTray(hModule, lpLibFileName);
    return hModule;
}

static bool HookTaskbarDllSymbols() {
    HMODULE hTaskbar = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!hTaskbar) { Wh_Log(L"[Hooks] Failed to load taskbar.dll"); return false; }
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        { {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"}, &CTaskBand_ITaskListWndSite_vftable },
        { {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"}, &CTaskBand_GetTaskbarHost_Original },
        { {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"}, &TaskbarHost_FrameHeight_Original },
        { {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"}, &std__Ref_count_base__Decref_Original },
    };
    return WindhawkUtils::HookSymbols(hTaskbar, hooks, ARRAYSIZE(hooks));
}

// ── Windhawk lifecycle ─────────────────────────────────────────────────────

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] OmniButton Customizer v1.0+colors");
    LoadSettings();
    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll symbol hooks failed — continuing");
    if (HMODULE hSysTray = GetSystemTrayModuleHandle()) {
        g_systemTrayModuleHooked = true;
        if (!HookSystemTraySymbols(hSysTray)) Wh_Log(L"[Init] system tray symbol hooks failed");
    } else {
        Wh_Log(L"[Init] System tray module not loaded yet");
        HMODULE kb = GetModuleHandleW(L"kernelbase.dll");
        auto pLoad = kb ? reinterpret_cast<LoadLibraryExW_t>(GetProcAddress(kb, "LoadLibraryExW")) : nullptr;
        if (pLoad) WindhawkUtils::SetFunctionHook(
            pLoad, LoadLibraryExW_Hook, &LoadLibraryExW_Original);
    }
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_systemTrayModuleHooked) {
        if (HMODULE hSysTray = GetSystemTrayModuleHandle()) {
            if (!g_systemTrayModuleHooked.exchange(true)) {
                Wh_Log(L"[AfterInit] system tray module found — hooking");
                if (HookSystemTraySymbols(hSysTray)) Wh_ApplyHookOperations();
            }
        }
    }
    if (g_systemTrayModuleHooked) ApplyAllSettingsOnWindowThread();
    Wh_Log(L"[AfterInit] systemTrayModuleHooked=%d", (int)g_systemTrayModuleHooked.load());
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        if (!RunFromWindowThread(hWnd, [](void*) {
            g_autoRevokerList.clear();
            CleanupAndResetCurrentElements();
        }, nullptr)) {
            g_autoRevokerList.clear();
            CleanupAndResetCurrentElements();
        }
    } else {
        g_autoRevokerList.clear();
        CleanupAndResetCurrentElements();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Updated");
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Settings] No taskbar window"); return; }
    if (!RunFromWindowThread(hWnd, [](void*) {
        CleanupAndResetCurrentElements();
        ApplyAllSettings();
    }, nullptr)) {
        CleanupAndResetCurrentElements();
        ApplyAllSettings();
    }
}
