// ==WindhawkMod==
// @id              tray-privacy-indicator-anchor
// @name            Tray Privacy Indicator Anchor
// @description     Permanently shows location/microphone/camera/Copilot icons in the system tray — dim when idle, bright when in use — preventing taskbar layout shifts.
// @version         0.9
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -lsetupapi -lcfgmgr32 -lshell32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Tray Privacy Indicator Anchor

A Windhawk mod for Windows 11 that reserves stable tray space for privacy and
status indicators. Location, microphone, camera, and Copilot placeholders stay
visible in the system tray: dim when idle, bright when active.

The goal is to stop taskbar tray sections from shifting when Windows briefly
shows or hides privacy indicators, especially when Windows Web Experience Pack
or Widgets frequently access location.

## Features

- Persistent placeholder icons for location, microphone, camera, and Copilot
- Idle opacity setting so inactive icons can be subtle but still reserve space
- Configurable icon order with `location`, `mic`, `camera`, and `copilot` tokens
- Row-first or column-first grid fill
- Short row/column placement and alignment controls
- Placement before icons, before OmniButton, before clock, after clock, or after Show Desktop
- Per-icon X/Y nudges plus whole-group X/Y offset
- Independent idle, active, disabled, glow, and slash colors
- Steady, breathing, or radiating active emphasis with reach/speed controls
- Disabled slash overlays for blocked or unavailable privacy devices
- Hardware camera shutter/kill-switch detection on supported Windows 11 camera drivers
- Evidence-specific tooltips instead of a generic "hardware disabled" label
- Click-through to the relevant Windows privacy, input, camera, taskbar, or app settings
- Optional testing toggle to let Windows' native privacy indicators appear

## Icon order and grid layout

`itemOrder` is a comma-separated list of icon tokens that controls which icons
appear and in what sequence: `location`, `mic`, `camera`, `copilot`. Remove a
token to hide that icon; reorder tokens to change the display order.

`gridRows` and `gridColumns` shape the icon grid; `0` (the default for both) is
automatic: a single column when the whole icon stack fits the taskbar height
(double-height taskbars), otherwise more columns. Set either one to fix that
axis — the other follows from the icon count.

When one row or column has fewer icons than the rest, use `shortGroupPosition`
and `shortGroupAlign` to control where it sits and how it's aligned:

| shortGroupPosition | shortGroupAlign | Result with location,mic,camera in 2 cols |
| --- | --- | --- |
| last (default) | center | `[loc  mic]` / `[  cam  ]` |
| first          | center | `[  loc  ]` / `[mic  cam]` |
| last           | start  | `[loc  mic]` / `[cam     ]` |
| last           | end    | `[loc  mic]` / `[     cam]` |

`fillOrder: columnFirst` fills columns instead of rows, giving vertical
arrangements like:

```
[loc] [cam]      [loc] [mic]
[mic] [   ]  or  [   ] [cam]   (short column centered)
```

## States and colors

Each icon has four visual states: idle/available, active, disabled/unavailable,
and active while disabled. The last state keeps the active treatment underneath
the slash by default, so a muted or shuttered device still demands attention
when Windows reports attempted use. `alertWhenBlockedAndActive` can turn that
combined treatment off.

`idleColor`, `activeColor`, `disabledColor`, `glowColor`, and `slashColor`
accept `#RRGGBB` or `#AARRGGBB` hex (the alpha byte is honored), the generics
`accent`, `accentLight`, and `accentDark` for the Windows accent shades, or
`transparent`. An empty color uses the system foreground, except an empty
`glowColor`, which follows `activeColor` or the Windows accent color.

The glow can be a steady halo, a breathing pulse, or animated radiation rings.
Its opacity, reach, and speed are independent controls. The effect is drawn
inside the existing icon slot and never changes the taskbar width.

For a deliberately striking treatment, start with `glowStyle: radiate`,
`glowOpacity: 85`, `glowSize: 260`, and `glowSpeed: 850`, then choose an
`activeColor`/`glowColor` that fits the rest of the taskbar theme.

## Files

- [privacy-indicator-anchor.wh.cpp](privacy-indicator-anchor.wh.cpp) - Windhawk mod source
- [privacy-trigger-test.html](privacy-trigger-test.html) - local browser test page for triggering privacy states
- [privacy-trigger-server.ps1](privacy-trigger-server.ps1) - helper server for the test page
- [privacy-diag.ps1](privacy-diag.ps1) - diagnostic helper for privacy/device state
- [archive/](archive/) - earlier experiments
- [assets/](assets/) - visual/test assets

## Status

Version `0.9` is still in lab development. Camera hardware-switch detection and
the Copilot indicator are experimental because Windows exposes those states
differently across devices and builds.

## Notes

Camera activity is detected from Windows webcam-usage records and any mirrored
native privacy state. Hardware camera blocking uses Windows 11
`CameraOcclusionInfo` when the camera driver supports it. Cameras without that
driver capability retain software-access and device-availability checks, but
their physical shutter or kill-switch state might not be detectable.

Windows documents idle-camera occlusion reports as advisory rather than an
absolute privacy guarantee. The tooltip therefore says "likely blocked" and
names the camera-driver evidence. `cameraHardwareDetection` can disable this
monitor. When enabled, it initializes the default camera controller in
`SharedReadOnly` mode but never starts preview or frame capture. Turn it off if
a particular camera activates its LED/indicator or behaves poorly while
monitored. State changes use the driver's native event; a five-minute watchdog
only checks that the subscription remains responsive. The controller is also
released when `camera` is removed from `itemOrder`.

Privacy access, usage records, policies, packages, and device topology are
monitored with Windows registry/device events rather than a three-second global
sweep. Copilot process activity is checked separately once per minute, and a
five-minute health reconciliation repairs any missed notification. Failed
camera and registry-monitor setup backs off from seconds to thirty minutes;
access-denied registry monitors remain disabled for the current mod session.

Each icon is clickable. Location opens Location privacy settings. Microphone
opens either microphone privacy or default-input settings according to the
reported reason. Camera opens either camera privacy or camera-device settings.
Copilot opens taskbar or installed-app settings.

`suppressNativeIndicators` defaults to `1` so the mod hides Windows' own pop-in
privacy indicators and mirrors state into the stable placeholders. Set it to `0`
temporarily when comparing against Windows' native tray glyphs during testing.

Run `privacy-diag.ps1 -Watch` while flipping hardware privacy switches to see
compact microphone and camera state changes without restarting the full report.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- position: "beforeOmni"
  $name: Position
  $description: Where to place the privacy placeholders in the tray.
  $options:
  - "beforeIcons": "Before notification icons"
  - "beforeOmni": "Before OmniButton (wifi/vol/bat)"
  - "beforeClock": "Before clock"
  - "afterClock": "After clock"
  - "afterShowDesktop": "After Show Desktop strip"

- itemOrder: "location,mic,camera,copilot"
  $name: Icon order
  $description: >-
    Comma-separated list of icons to show, in order. Valid tokens: location,
    mic, camera, copilot. Remove a token to hide that icon. Reorder to change
    layout. Camera and copilot are experimental — see mod description.

- cameraHardwareDetection: true
  $name: Monitor camera hardware privacy control
  $description: >-
    Uses the Windows 11 CameraOcclusionInfo driver signal. This initializes the
    default camera controller in SharedReadOnly mode but never starts preview or
    frame capture. State changes are event-driven with a five-minute watchdog.
    Turn off if a particular camera activates its LED/indicator or behaves
    poorly while monitored.

- gridRows: 0
  $name: Rows (0 = auto)
  $description: >-
    Rows in the icon grid. 0 = automatic from the icon count and taskbar
    height (see Columns).

- gridColumns: 0
  $name: Columns (0 = auto)
  $description: >-
    Columns in the icon grid. 0 (default) picks automatically: a single column
    when the whole icon stack fits the taskbar height (double-height
    taskbars), otherwise more columns.

- fillOrder: "rowFirst"
  $name: Fill order
  $options:
  - "rowFirst": "Row first"
  - "columnFirst": "Column first"

- shortGroupPosition: "last"
  $name: Short row or column
  $description: When icons don't divide evenly, where the short row/column goes.
  $options:
  - "first": "First"
  - "last": "Last"

- shortGroupAlign: "center"
  $name: Short row or column alignment
  $options:
  - "start": "Start"
  - "center": "Center"
  - "end": "End"

- iconSize: 16
  $name: Icon size (pt)

- buttonSpacing: 4
  $name: Icon spacing (px)
  $description: Gap between icons in both directions.

- idleOpacity: 50
  $name: Idle opacity (0-100)
  $description: >-
    Opacity when no app is using the feature. 0 = invisible but space reserved;
    100 = always full brightness.

- idleColor: ""
  $name: Idle icon color
  $description: >-
    Color used while the feature is available but idle. Hex ("#RRGGBB" or
    "#AARRGGBB"), "accent" / "accentLight" / "accentDark", or "transparent".
    Empty (default) keeps the system foreground color.

- activeColor: ""
  $name: Active icon color
  $description: >-
    Color applied to icons while their feature is in use. Hex ("#RRGGBB" or
    "#AARRGGBB"), "accent" / "accentLight" / "accentDark", or "transparent".
    Empty (default) keeps the system foreground color at full brightness.

- disabledOpacity: 50
  $name: Disabled icon opacity (0-100)
  $description: >-
    Base-icon opacity when access, a device, or a service is unavailable. The
    slash has its own opacity control.

- disabledColor: ""
  $name: Disabled icon color
  $description: >-
    Base-icon color while unavailable. Hex ("#RRGGBB" or "#AARRGGBB"),
    "accent" / "accentLight" / "accentDark", or "transparent". Empty
    (default) keeps the system foreground color.

- alertWhenBlockedAndActive: 1
  $name: Emphasize blocked activity (1=on, 0=off)
  $description: >-
    When Windows reports use while the feature is blocked, keep the active
    color and glow beneath the slash. Turn off to use ordinary disabled styling.

- glowEnabled: 0
  $name: Glow when active (1=on, 0=off)
  $description: >-
    Adds a halo or radiating emphasis behind the icon while active. The effect
    stays inside the reserved icon slot and does not change taskbar width.

- glowStyle: "radiate"
  $name: Glow style
  $options:
  - "steady": "Steady halo"
  - "pulse": "Breathing halo"
  - "radiate": "Radiating rings"

- glowColor: ""
  $name: Glow color
  $description: >-
    Hex ("#RRGGBB" or "#AARRGGBB"), "accent" / "accentLight" /
    "accentDark", or "transparent". Empty follows Active icon color when set,
    otherwise it uses the Windows accent color.

- glowOpacity: 40
  $name: Glow opacity (0-100)
  $description: Peak strength of the halo and radiation rings.

- glowSize: 220
  $name: Glow reach (percent)
  $description: >-
    Maximum glow diameter as a percentage of icon size. Range: 100-300.

- glowSpeed: 1200
  $name: Glow cycle (ms)
  $description: >-
    Animation cycle time in milliseconds. Range is 250-5000.

- slashColor: ""
  $name: Slash color
  $description: >-
    Color of the slash overlay shown when a feature is disabled. Hex
    ("#RRGGBB" or "#AARRGGBB"), "accent" / "accentLight" / "accentDark", or
    "transparent". Leave empty (default) to use the system foreground color,
    matching the dimmed icon.

- slashDirection: "falling"
  $name: Slash direction
  $description: >-
    Direction of the diagonal line drawn through the icon when disabled.
    Falling (the default) avoids visually colliding with the icon glyphs.
  $options:
  - "falling": "Falling (\ upper-left to lower-right)"
  - "rising": "Rising (/ lower-left to upper-right)"

- slashOpacity: 100
  $name: Slash opacity (0-100)
  $description: >-
    Opacity of the disabled slash overlay. 100 = fully visible (default).
    Lower values make the slash more subtle.

- groupPaddingLeft: 0
  $name: Group padding left (px)

- groupPaddingRight: 0
  $name: Group padding right (px)

- groupOffsetX: 0
  $name: Group X offset (px)
  $description: Move the entire icon group left (negative) or right (positive).

- groupOffsetY: 0
  $name: Group Y offset (px)
  $description: Move the entire icon group up (negative) or down (positive).

- locationOffsetX: 0
  $name: Location X offset (px)

- locationOffsetY: 0
  $name: Location Y offset (px)

- micOffsetX: 0
  $name: Microphone X offset (px)

- micOffsetY: 0
  $name: Microphone Y offset (px)

- cameraOffsetX: 0
  $name: Camera X offset (px)

- cameraOffsetY: 0
  $name: Camera Y offset (px)

- copilotOffsetX: 0
  $name: Copilot X offset (px)

- copilotOffsetY: 0
  $name: Copilot Y offset (px)

- suppressNativeIndicators: 1
  $name: Suppress Windows privacy indicators (1=on, 0=off)
  $description: >-
    When on, hides Windows' own pop-in privacy indicators and mirrors their
    state into this mod's stable placeholder icons. Turn off temporarily when
    testing Windows' native glyphs and tray behavior.
*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Capture.h>
#include <winrt/Windows.Media.Devices.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Shapes.h>

#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <cfgmgr32.h>
#include <shellapi.h>
#include <setupapi.h>
#include <tlhelp32.h>

#include <algorithm>
#include <atomic>
#include <cwctype>
#include <functional>
#include <list>
#include <string>
#include <vector>

#include <windhawk_utils.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Smart grid layout
// Template block: _templates/smart-grid-layout.h v1.0 (verbatim copy — keep
// in sync with the template; Windhawk mods are single-file).
// ============================================================

#include <climits>

namespace windhawk_mod_templates::smart_grid {

enum class GridMode {
    AutoSmart,
    SingleRow,
    SingleColumn,
    FixedRows,
    FixedColumns,
    FixedGrid,
};

enum class SmartLayout { Balanced, PackVertical, PackHorizontal };
enum class FillOrder { RowFirst, ColumnFirst };
enum class ShortGroupPosition { First, Last };
enum class ShortGroupAlign { Start, Center, End };

struct Config {
    GridMode mode = GridMode::AutoSmart;
    SmartLayout smartLayout = SmartLayout::Balanced;
    FillOrder fillOrder = FillOrder::RowFirst;
    ShortGroupPosition shortGroupPosition = ShortGroupPosition::Last;
    ShortGroupAlign shortGroupAlign = ShortGroupAlign::Center;
    int rows = 0;          // exact in fixed modes; maximum in AutoSmart
    int columns = 0;       // exact in fixed modes; maximum in AutoSmart
    int availableRows = 1; // derive from host height / item pitch
};

struct Layout {
    int rows = 1;
    int columns = 1;
};

// A short group may need to span its complete axis so a half-cell offset can
// be expressed with Margin. Multiply offsetUnits by item-size-plus-spacing.
struct Cell {
    int row = 0;
    int column = 0;
    int rowSpan = 1;
    int columnSpan = 1;
    double topOffsetUnits = 0.0;
    double leftOffsetUnits = 0.0;
};

inline int ScoreCandidate(int rows, int columns, int count,
                          SmartLayout preference) {
    int waste = rows * columns - count;
    int widePenalty = columns > rows ? (columns - rows) * 2 : 0;
    int score = waste * 10 + widePenalty;

    if (preference == SmartLayout::PackVertical)
        score -= rows * 20;
    else if (preference == SmartLayout::PackHorizontal)
        score += rows * 20;
    else
        score -= rows * 3;

    return score;
}

inline Layout ComputeLayout(int count, Config const& config) {
    count = std::max(1, count);
    Layout result;
    int availableRows = std::clamp(config.availableRows, 1, count);
    if (config.rows > 0 && config.mode == GridMode::AutoSmart)
        availableRows = std::min(availableRows, config.rows);

    switch (config.mode) {
        case GridMode::SingleRow:
            result = {1, count};
            break;
        case GridMode::SingleColumn:
            result = {count, 1};
            break;
        case GridMode::FixedRows:
            result.rows = std::clamp(config.rows, 1, count);
            result.columns = (count + result.rows - 1) / result.rows;
            break;
        case GridMode::FixedColumns:
            result.columns = std::clamp(config.columns, 1, count);
            result.rows = (count + result.columns - 1) / result.columns;
            break;
        case GridMode::FixedGrid:
            result.rows = std::clamp(config.rows, 1, count);
            result.columns = config.columns > 0
                ? std::clamp(config.columns, 1, count)
                : (count + result.rows - 1) / result.rows;
            if (result.rows * result.columns < count)
                result.rows = (count + result.columns - 1) / result.columns;
            break;
        case GridMode::AutoSmart: {
            int bestScore = INT_MAX;
            int firstRows = availableRows > 1 && count > 1 &&
                            config.smartLayout != SmartLayout::PackHorizontal
                ? 2 : 1;
            for (int rows = firstRows; rows <= availableRows; ++rows) {
                int columns = (count + rows - 1) / rows;
                if (config.columns > 0 && columns > config.columns)
                    continue;
                int score = ScoreCandidate(rows, columns, count,
                                           config.smartLayout);
                if (score < bestScore) {
                    bestScore = score;
                    result = {rows, columns};
                }
            }
            if (bestScore == INT_MAX) {
                result.columns = std::clamp(config.columns, 1, count);
                result.rows = (count + result.columns - 1) / result.columns;
            }
            break;
        }
    }

    result.rows = std::clamp(result.rows, 1, count);
    result.columns = std::max(1, result.columns);
    while (result.rows * result.columns < count) {
        if (config.mode == GridMode::FixedColumns)
            ++result.rows;
        else
            ++result.columns;
    }
    return result;
}

inline double AlignOffset(int capacity, int itemCount,
                          ShortGroupAlign alignment) {
    int unused = std::max(0, capacity - itemCount);
    if (alignment == ShortGroupAlign::Center)
        return unused / 2.0;
    if (alignment == ShortGroupAlign::End)
        return static_cast<double>(unused);
    return 0.0;
}

inline Cell GetCell(int index, int count, Layout const& layout,
                    Config const& config) {
    Cell cell;
    index = std::clamp(index, 0, std::max(0, count - 1));

    if (config.fillOrder == FillOrder::RowFirst) {
        int groupCount = (count + layout.columns - 1) / layout.columns;
        int shortCount = count % layout.columns;
        if (!shortCount) shortCount = layout.columns;
        int group;
        int itemInGroup;
        if (shortCount < layout.columns &&
            config.shortGroupPosition == ShortGroupPosition::First) {
            if (index < shortCount) {
                group = 0;
                itemInGroup = index;
            } else {
                int adjusted = index - shortCount;
                group = 1 + adjusted / layout.columns;
                itemInGroup = adjusted % layout.columns;
            }
        } else {
            group = index / layout.columns;
            itemInGroup = index % layout.columns;
        }
        int shortGroup = config.shortGroupPosition == ShortGroupPosition::First
            ? 0 : groupCount - 1;
        bool isShort = shortCount < layout.columns && group == shortGroup;

        cell.row = group;
        cell.column = itemInGroup;
        if (isShort && config.shortGroupAlign != ShortGroupAlign::Start) {
            cell.column = 0;
            cell.columnSpan = layout.columns;
            cell.leftOffsetUnits = AlignOffset(layout.columns, shortCount,
                                               config.shortGroupAlign) +
                                   itemInGroup;
        }
    } else {
        int groupCount = (count + layout.rows - 1) / layout.rows;
        int shortCount = count % layout.rows;
        if (!shortCount) shortCount = layout.rows;
        int group;
        int itemInGroup;
        if (shortCount < layout.rows &&
            config.shortGroupPosition == ShortGroupPosition::First) {
            if (index < shortCount) {
                group = 0;
                itemInGroup = index;
            } else {
                int adjusted = index - shortCount;
                group = 1 + adjusted / layout.rows;
                itemInGroup = adjusted % layout.rows;
            }
        } else {
            group = index / layout.rows;
            itemInGroup = index % layout.rows;
        }
        int shortGroup = config.shortGroupPosition == ShortGroupPosition::First
            ? 0 : groupCount - 1;
        bool isShort = shortCount < layout.rows && group == shortGroup;

        cell.row = itemInGroup;
        cell.column = group;
        if (isShort && config.shortGroupAlign != ShortGroupAlign::Start) {
            cell.row = 0;
            cell.rowSpan = layout.rows;
            cell.topOffsetUnits = AlignOffset(layout.rows, shortCount,
                                              config.shortGroupAlign) +
                                  itemInGroup;
        }
    }
    return cell;
}

} // namespace windhawk_mod_templates::smart_grid

namespace grid = windhawk_mod_templates::smart_grid;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    std::wstring position = L"beforeOmni";
    std::wstring itemOrder = L"location,mic,camera,copilot";
    bool cameraHardwareDetection = true;
    int  gridRows    = 0;
    int  gridColumns = 0;
    grid::FillOrder          fillOrder          = grid::FillOrder::RowFirst;
    grid::ShortGroupPosition shortGroupPosition = grid::ShortGroupPosition::Last;
    grid::ShortGroupAlign    shortGroupAlign    = grid::ShortGroupAlign::Center;
    int  iconSize      = 16;
    int  buttonSpacing = 4;
    int  idleOpacity   = 50;
    bool idleColorSet = false;
    winrt::Windows::UI::Color idleColorValue{};
    int  disabledOpacity = 50;
    bool disabledColorSet = false;
    winrt::Windows::UI::Color disabledColorValue{};
    bool alertWhenBlockedAndActive = true;
    int  groupPaddingLeft = 0;
    int  groupPaddingRight = 0;
    int  groupOffsetX = 0;
    int  groupOffsetY = 0;
    int  locationOffsetX = 0;
    int  locationOffsetY = 0;
    int  micOffsetX   = 0;
    int  micOffsetY   = 0;
    int  cameraOffsetX = 0;
    int  cameraOffsetY = 0;
    int  copilotOffsetX = 0;
    int  copilotOffsetY = 0;
    bool activeColorSet = false;
    winrt::Windows::UI::Color activeColorValue{};
    bool glowEnabled    = false;
    std::wstring glowStyle = L"radiate";
    bool glowColorSet = false;
    winrt::Windows::UI::Color glowColorValue{};
    int  glowOpacity    = 40;
    int  glowSize       = 220;
    int  glowSpeed      = 1200;
    bool slashColorSet  = false;   // false = system theme
    winrt::Windows::UI::Color slashColorValue{};
    std::wstring slashDirection = L"falling";
    int  slashOpacity   = 100;
    bool suppressNativeIndicators = true;
};
static ModSettings g_settings;
static std::atomic<bool> g_cameraHardwareDetectionEnabled{true};
static std::atomic<bool> g_cameraItemEnabled{true};
static std::atomic<bool> g_copilotItemEnabled{true};

static std::vector<std::wstring> ParseItemOrder(std::wstring const& s);

static std::wstring GetStringSetting(PCWSTR name) {
    PCWSTR raw = Wh_GetStringSetting(name);
    std::wstring value = raw;
    Wh_FreeStringSetting(raw);
    return value;
}

// Color-returning variant of the canonical token parser (_templates/button-surface.h):
// "#RRGGBB" / "#AARRGGBB" hex (alpha honored, "#" required), the generics
// "accent" / "accentLight" / "accentDark" / "transparent", and the numbered
// Windows shades "accentLight1"-"3" / "accentDark1"-"3" (accepted silently,
// undocumented). Empty/unparseable returns false = keep the native behavior.
static bool ParseColorToken(const wchar_t* s, winrt::Windows::UI::Color& out) {
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

    if (*s != L'#') return false;
    const wchar_t* p = s + 1;
    size_t len = wcslen(p);
    if (len != 6 && len != 8) return false;
    for (size_t i = 0; i < len; i++)
        if (!iswxdigit(p[i])) return false;
    unsigned long v = wcstoul(p, nullptr, 16);
    if (len == 6) { out = {255, BYTE(v>>16), BYTE(v>>8), BYTE(v)}; }
    else          { out = {BYTE(v>>24), BYTE(v>>16), BYTE(v>>8), BYTE(v)}; }
    return true;
}

static void LoadSettings() {
    auto clamp = [](int v, int lo, int hi) { return std::max(lo, std::min(hi, v)); };
    g_settings.position  = GetStringSetting(L"position");
    g_settings.itemOrder = GetStringSetting(L"itemOrder");
    g_settings.cameraHardwareDetection =
        Wh_GetIntSetting(L"cameraHardwareDetection") != 0;
    g_cameraHardwareDetectionEnabled.store(
        g_settings.cameraHardwareDetection);
    auto enabledItems = ParseItemOrder(g_settings.itemOrder);
    g_cameraItemEnabled.store(
        std::find(enabledItems.begin(), enabledItems.end(), L"camera") !=
        enabledItems.end());
    g_copilotItemEnabled.store(
        std::find(enabledItems.begin(), enabledItems.end(), L"copilot") !=
        enabledItems.end());
    g_settings.gridRows    = clamp(Wh_GetIntSetting(L"gridRows"), 0, 10);
    g_settings.gridColumns = clamp(Wh_GetIntSetting(L"gridColumns"), 0, 10);
    { std::wstring s = GetStringSetting(L"fillOrder");
      // "colFirst" accepted as a legacy spelling of "columnFirst"
      g_settings.fillOrder = (s == L"columnFirst" || s == L"colFirst")
                           ? grid::FillOrder::ColumnFirst : grid::FillOrder::RowFirst; }
    { std::wstring s = GetStringSetting(L"shortGroupPosition");
      g_settings.shortGroupPosition = (s == L"first")
                                    ? grid::ShortGroupPosition::First
                                    : grid::ShortGroupPosition::Last; }
    { std::wstring s = GetStringSetting(L"shortGroupAlign");
      if      (s == L"start") g_settings.shortGroupAlign = grid::ShortGroupAlign::Start;
      else if (s == L"end")   g_settings.shortGroupAlign = grid::ShortGroupAlign::End;
      else                    g_settings.shortGroupAlign = grid::ShortGroupAlign::Center; }
    g_settings.iconSize             = clamp(Wh_GetIntSetting(L"iconSize"), 8, 48);
    g_settings.idleOpacity          = clamp(Wh_GetIntSetting(L"idleOpacity"), 0, 100);
    g_settings.disabledOpacity      = clamp(Wh_GetIntSetting(L"disabledOpacity"), 0, 100);
    g_settings.alertWhenBlockedAndActive =
        Wh_GetIntSetting(L"alertWhenBlockedAndActive") != 0;
    g_settings.groupPaddingLeft     = clamp(Wh_GetIntSetting(L"groupPaddingLeft"), -40, 40);
    g_settings.groupPaddingRight    = clamp(Wh_GetIntSetting(L"groupPaddingRight"), -40, 40);
    g_settings.buttonSpacing        = clamp(Wh_GetIntSetting(L"buttonSpacing"), 0, 40);
    g_settings.groupOffsetX         = clamp(Wh_GetIntSetting(L"groupOffsetX"), -40, 40);
    g_settings.groupOffsetY         = clamp(Wh_GetIntSetting(L"groupOffsetY"), -40, 40);
    g_settings.locationOffsetX      = clamp(Wh_GetIntSetting(L"locationOffsetX"), -40, 40);
    g_settings.locationOffsetY      = clamp(Wh_GetIntSetting(L"locationOffsetY"), -40, 40);
    g_settings.micOffsetX           = clamp(Wh_GetIntSetting(L"micOffsetX"), -40, 40);
    g_settings.micOffsetY           = clamp(Wh_GetIntSetting(L"micOffsetY"), -40, 40);
    g_settings.cameraOffsetX        = clamp(Wh_GetIntSetting(L"cameraOffsetX"), -40, 40);
    g_settings.cameraOffsetY        = clamp(Wh_GetIntSetting(L"cameraOffsetY"), -40, 40);
    g_settings.copilotOffsetX       = clamp(Wh_GetIntSetting(L"copilotOffsetX"), -40, 40);
    g_settings.copilotOffsetY       = clamp(Wh_GetIntSetting(L"copilotOffsetY"), -40, 40);
    g_settings.glowEnabled          = Wh_GetIntSetting(L"glowEnabled") != 0;
    g_settings.glowOpacity          = clamp(Wh_GetIntSetting(L"glowOpacity"), 0, 100);
    g_settings.glowSize             = clamp(Wh_GetIntSetting(L"glowSize"), 100, 300);
    g_settings.glowSpeed            = clamp(Wh_GetIntSetting(L"glowSpeed"), 250, 5000);
    { std::wstring s = GetStringSetting(L"glowStyle");
      g_settings.glowStyle = (s == L"steady" || s == L"pulse")
                           ? s : L"radiate"; }
    g_settings.idleColorSet = ParseColorToken(
        GetStringSetting(L"idleColor").c_str(), g_settings.idleColorValue);
    g_settings.activeColorSet = ParseColorToken(
        GetStringSetting(L"activeColor").c_str(), g_settings.activeColorValue);
    g_settings.disabledColorSet = ParseColorToken(
        GetStringSetting(L"disabledColor").c_str(), g_settings.disabledColorValue);
    g_settings.glowColorSet = ParseColorToken(
        GetStringSetting(L"glowColor").c_str(), g_settings.glowColorValue);
    g_settings.slashColorSet = ParseColorToken(
        GetStringSetting(L"slashColor").c_str(), g_settings.slashColorValue);
    g_settings.slashDirection = GetStringSetting(L"slashDirection");
    g_settings.slashOpacity   = clamp(Wh_GetIntSetting(L"slashOpacity"), 0, 100);
    g_settings.suppressNativeIndicators = Wh_GetIntSetting(L"suppressNativeIndicators") != 0;
}

// ============================================================
// Globals
// ============================================================

enum class PrivacyBlockReason {
    None,
    UserAccessDenied,
    SystemAccessDenied,
    PolicyDisabled,
    ServiceDisabled,
    EndpointMuted,
    DeviceDisabled,
    DeviceUnavailable,
    CameraHardwareOcclusion,
    NotInstalled,
    TaskbarSettingOff,
};

enum class PrivacyItemKind { Location, Microphone, Camera, Copilot };

enum StateRefreshFlags : DWORD {
    RefreshNone             = 0,
    RefreshLocationState    = 1u << 0,
    RefreshMicrophoneState  = 1u << 1,
    RefreshCameraState      = 1u << 2,
    RefreshLocationUsage    = 1u << 3,
    RefreshMicrophoneUsage  = 1u << 4,
    RefreshCameraUsage      = 1u << 5,
    RefreshCopilotState     = 1u << 6,
    RefreshCopilotActivity  = 1u << 7,
    RefreshMonitorSetup     = 1u << 8,
    RefreshAll = RefreshLocationState | RefreshMicrophoneState |
                 RefreshCameraState | RefreshLocationUsage |
                 RefreshMicrophoneUsage | RefreshCameraUsage |
                 RefreshCopilotState | RefreshCopilotActivity,
};

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd           = nullptr;
static std::atomic<bool>  g_systemTrayModuleHooked{false};
static HANDLE            g_retryThread          = nullptr;
static HANDLE            g_retryStopEvent       = nullptr;
static HANDLE            g_stateRefreshEvent    = nullptr;
static std::atomic<DWORD> g_pendingRefreshFlags{RefreshAll};

static void RequestStateRefresh(DWORD flags) {
    g_pendingRefreshFlags.fetch_or(flags);
    if (g_stateRefreshEvent)
        SetEvent(g_stateRefreshEvent);
}

static std::atomic<bool> g_locActive{false};
static std::atomic<bool> g_micActive{false};
static std::atomic<bool> g_camActive{false};
// Usage-record detection (ConsentStore LastUsedTimeStop==0) — covers hardware
// camera/mic/location that never get a native tray glyph. ORed with the
// glyph-driven *Active flags above when rendering.
static std::atomic<bool> g_locUsage{false};
static std::atomic<bool> g_micUsage{false};
static std::atomic<bool> g_camUsage{false};
static std::atomic<bool> g_locDisabled{false};
static std::atomic<bool> g_micDisabled{false};
static std::atomic<bool> g_camDisabled{false};
static std::atomic<PrivacyBlockReason> g_locBlockReason{PrivacyBlockReason::None};
static std::atomic<PrivacyBlockReason> g_micBlockReason{PrivacyBlockReason::None};
static std::atomic<PrivacyBlockReason> g_camBlockReason{PrivacyBlockReason::None};
static std::atomic<PrivacyBlockReason> g_copilotBlockReason{PrivacyBlockReason::NotInstalled};
static std::atomic<bool> g_cameraOcclusionSupported{false};
static std::atomic<bool> g_cameraHardwareOccluded{false};
static std::atomic<bool> g_copilotInstalled{false};
static std::atomic<bool> g_copilotActive{false};
static std::atomic<bool> g_copilotDisabled{true};
static Grid              g_syntheticGrid   = nullptr;
static FrameworkElement  g_locIcon         = nullptr;
static FrameworkElement  g_micIcon         = nullptr;
static FrameworkElement  g_camIcon         = nullptr;
static FrameworkElement  g_copilotIcon     = nullptr;
static FrameworkElement  g_locSlot         = nullptr;
static FrameworkElement  g_micSlot         = nullptr;
static FrameworkElement  g_camSlot         = nullptr;
static FrameworkElement  g_copilotSlot     = nullptr;
static FrameworkElement  g_locGlowIcon     = nullptr;
static FrameworkElement  g_micGlowIcon     = nullptr;
static FrameworkElement  g_camGlowIcon     = nullptr;
static FrameworkElement  g_copilotGlowIcon = nullptr;
static FrameworkElement  g_locSlashIcon    = nullptr;
static FrameworkElement  g_micSlashIcon    = nullptr;
static FrameworkElement  g_camSlashIcon    = nullptr;
static FrameworkElement  g_copilotSlashIcon = nullptr;
static FrameworkElement  g_syntheticParent = nullptr;
static int               g_syntheticColumn = -1;

struct SlotEventState {
    FrameworkElement element{nullptr};
    winrt::event_token tappedToken{};
};
[[clang::no_destroy]] static std::vector<SlotEventState> g_slotEventStates;

struct GlowAnimationState {
    FrameworkElement element{nullptr};
    std::vector<winrt::Windows::UI::Xaml::Media::Animation::Storyboard>
        storyboards;
    bool running = false;
};
[[clang::no_destroy]] static std::vector<GlowAnimationState>
    g_glowAnimationStates;

struct PrivacyState {
    enum class Type { Location, Mic, Camera, Both };
    winrt::weak_ref<FrameworkElement> iconViewRef;
    winrt::weak_ref<TextBlock>        textBlockRef;
    int64_t textToken       = 0;
    int64_t visibilityToken = 0;
    Type    type      = Type::Location;
};
static std::vector<PrivacyState> g_privacyStates;

using FrameworkElementLoadedRevoker = winrt::impl::event_revoker<
    IFrameworkElement,
    &winrt::impl::abi<IFrameworkElement>::type::remove_Loaded>;
static std::list<FrameworkElementLoadedRevoker> g_loadedRevokers;

// Forward declarations
static void ApplyStyle();
static void ApplyStyleOnWindowThread();
static void ClearPrivacyStates();
static void RemoveSyntheticIcons();
static void StopRetryThread();
static void UpdatePrivacyStates(DWORD flags);
static bool HookSystemTraySymbols(HMODULE h);
static void HandleLoadedModuleIfSystemTray(HMODULE module,
                                            LPCWSTR fileName);

static VS_FIXEDFILEINFO* GetModuleVersionInfo(HMODULE module) {
    void* fixedFileInfo = nullptr;
    UINT length = 0;
    HRSRC resource = FindResourceW(
        module, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
    if (!resource) return nullptr;
    HGLOBAL loaded = LoadResource(module, resource);
    void* data = loaded ? LockResource(loaded) : nullptr;
    if (!data || !VerQueryValueW(data, L"\\", &fixedFileInfo, &length) ||
        !length)
        return nullptr;
    return static_cast<VS_FIXEDFILEINFO*>(fixedFileInfo);
}

static HMODULE GetSystemTrayModuleHandle() {
    HMODULE module = GetModuleHandleW(L"SystemTray.dll");
    if (!module) {
        module = GetModuleHandleW(L"Taskbar.View.dll");
        if (module) {
            auto version = GetModuleVersionInfo(module);
            WORD major = version ? HIWORD(version->dwFileVersionMS) : 0;
            if (!major || major >= 2604)
                module = nullptr;
        }
    }
    if (!module)
        module = GetModuleHandleW(L"ExplorerExtensions.dll");
    return module;
}

// ============================================================
// GetTaskbarXamlRoot
// ============================================================

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void* pThis, void* result);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using TaskbarHost_FrameHeight_t = int (WINAPI*)(void* pThis);
TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;

using std__Ref_count_base__Decref_t = void (WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    if (!CTaskBand_GetTaskbarHost_Original ||
        !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable)
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
    if (!taskbarHostSharedPtr[0] || !taskbarHostSharedPtr[1]) {
        if (taskbarHostSharedPtr[1])
            std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }
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

// ============================================================
// Window thread marshalling / taskbar discovery
// ============================================================

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
            if (cwp->message == RegisterWindowMessageW(L"Windhawk_RunFromWindowThread_" WH_MOD_ID)) {
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

// ============================================================
// XAML helpers
// ============================================================

static FrameworkElement FindChildByName(FrameworkElement element, PCWSTR name) {
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && child.Name() == name) return child;
    }
    return nullptr;
}

static FrameworkElement FindChildByClassName(FrameworkElement element, PCWSTR className) {
    int n = VisualTreeHelper::GetChildrenCount(element);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(element, i).try_as<FrameworkElement>();
        if (child && winrt::get_class_name(child) == className) return child;
    }
    return nullptr;
}

static FrameworkElement FindChildRecursive(FrameworkElement const& element,
    std::function<bool(FrameworkElement)> const& cb, int maxDepth = 20) {
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

// ============================================================
// Privacy type detection
// ============================================================

static PrivacyState::Type DetectPrivacyType(std::wstring_view text) {
    if (text.empty()) return PrivacyState::Type::Location;
    switch (text[0]) {
        case 0xE37A: return PrivacyState::Type::Location;
        case 0xF47F: return PrivacyState::Type::Both;
        case 0xE361:
        case 0xE720:
        case 0xEC71: return PrivacyState::Type::Mic;
        case 0xE722: return PrivacyState::Type::Camera;
        default:     return PrivacyState::Type::Location;
    }
}

static bool IsPrivacyGlyph(wchar_t c) {
    return c == 0xE37A || c == 0xF47F ||
           c == 0xE361 || c == 0xE720 || c == 0xEC71 ||
           c == 0xE722;
}

static bool IsPrivacyText(std::wstring_view text) {
    return text.empty() || (text.length() == 1 && IsPrivacyGlyph(text[0]));
}

// ============================================================
// Icon order parsing
// ============================================================

static std::vector<std::wstring> ParseItemOrder(std::wstring const& s) {
    std::vector<std::wstring> result;
    size_t start = 0;
    while (start <= s.size()) {
        size_t end = s.find(L',', start);
        if (end == std::wstring::npos) end = s.size();
        size_t ts = start, te = end;
        while (ts < te && std::iswspace(s[ts])) ts++;
        while (te > ts && std::iswspace(s[te - 1])) te--;
        if (ts < te) {
            std::wstring token = s.substr(ts, te - ts);
            for (auto& c : token) c = std::towlower(c);
            if (token == L"location" || token == L"mic" || token == L"camera" || token == L"copilot") {
                // Deduplicate
                bool already = false;
                for (auto& r : result) if (r == token) { already = true; break; }
                if (!already) result.push_back(token);
            }
        }
        if (end == s.size()) break;
        start = end + 1;
    }
    return result;
}

// ============================================================
// Synthetic icon management
// ============================================================

static void SetGlowActive(FrameworkElement const& glow, bool active) {
    if (!glow) return;
    glow.Visibility(active ? Visibility::Visible : Visibility::Collapsed);
    for (auto& state : g_glowAnimationStates) {
        if (state.element != glow || state.running == active) continue;
        try {
            for (auto const& storyboard : state.storyboards) {
                if (active) storyboard.Begin();
                else        storyboard.Stop();
            }
            state.running = active;
        } catch (...) {
            Wh_Log(L"[Glow] Failed to %s animation",
                   active ? L"start" : L"stop");
        }
        break;
    }
}

static void UpdateSyntheticOpacity() {
    double idleOpacity = g_settings.idleOpacity / 100.0;
    double disabledOpacity = g_settings.disabledOpacity / 100.0;
    bool isDark =
        Application::Current().RequestedTheme() == ApplicationTheme::Dark;
    winrt::Windows::UI::Color neutralColor = isDark
        ? winrt::Windows::UI::Color{255, 255, 255, 255}
        : winrt::Windows::UI::Color{255, 30, 30, 30};

    // setShapeFill covers the Copilot Viewbox/Path and Polygon fallback.
    auto setShapeFill = [](FrameworkElement fe, winrt::Windows::UI::Color color) {
        SolidColorBrush br; br.Color(color);
        if (auto shape = fe.try_as<winrt::Windows::UI::Xaml::Shapes::Shape>()) {
            shape.Fill(br); return;
        }
        // VisualTreeHelper fallback (requires mounted tree — works during runtime updates)
        auto child = FindChildRecursive(fe,
            [](FrameworkElement e) { return e.try_as<winrt::Windows::UI::Xaml::Shapes::Shape>() != nullptr; });
        if (auto cs = child.try_as<winrt::Windows::UI::Xaml::Shapes::Shape>()) cs.Fill(br);
    };

    auto applyColor = [&](FrameworkElement const& icon, bool colorSet,
                          winrt::Windows::UI::Color color) {
        if (auto tb = icon.try_as<TextBlock>()) {
            if (colorSet) {
                SolidColorBrush brush; brush.Color(color);
                tb.Foreground(brush);
            } else {
                tb.ClearValue(TextBlock::ForegroundProperty());
            }
        } else {
            // Shape.Fill defaults to null, so explicitly restore a neutral
            // foreground when no custom color is selected.
            setShapeFill(icon, colorSet ? color : neutralColor);
        }
    };

    auto applySlot = [&](FrameworkElement icon, FrameworkElement glow, FrameworkElement slash,
                         bool active, bool disabled) {
        if (!icon) return;
        bool emphasizedActivity = active &&
            (!disabled || g_settings.alertWhenBlockedAndActive);

        if (emphasizedActivity) {
            icon.Opacity(1.0);
            applyColor(icon, g_settings.activeColorSet,
                       g_settings.activeColorValue);
        } else if (disabled) {
            icon.Opacity(disabledOpacity);
            applyColor(icon, g_settings.disabledColorSet,
                       g_settings.disabledColorValue);
        } else {
            icon.Opacity(idleOpacity);
            applyColor(icon, g_settings.idleColorSet,
                       g_settings.idleColorValue);
        }

        SetGlowActive(glow, emphasizedActivity && g_settings.glowEnabled);
        if (slash)
            slash.Visibility(disabled ? Visibility::Visible : Visibility::Collapsed);
    };

    applySlot(g_locIcon, g_locGlowIcon, g_locSlashIcon,
              g_locActive.load() || g_locUsage.load(), g_locDisabled.load());
    applySlot(g_micIcon, g_micGlowIcon, g_micSlashIcon,
              g_micActive.load() || g_micUsage.load(), g_micDisabled.load());
    applySlot(g_camIcon, g_camGlowIcon, g_camSlashIcon,
              g_camActive.load() || g_camUsage.load(), g_camDisabled.load());
    applySlot(g_copilotIcon, g_copilotGlowIcon, g_copilotSlashIcon,
              g_copilotActive.load(), g_copilotDisabled.load());
}

static std::wstring DescribeBlockReason(PrivacyBlockReason reason) {
    switch (reason) {
        case PrivacyBlockReason::UserAccessDenied:
            return L"Blocked - access denied in Windows privacy settings";
        case PrivacyBlockReason::SystemAccessDenied:
            return L"Blocked - access denied by Windows or an administrator";
        case PrivacyBlockReason::PolicyDisabled:
            return L"Blocked by system policy";
        case PrivacyBlockReason::ServiceDisabled:
            return L"Off - the Windows location service is disabled";
        case PrivacyBlockReason::EndpointMuted:
            return L"Muted - the default recording endpoint reports mute\n"
                   L"Evidence: Windows audio endpoint state (software/firmware)";
        case PrivacyBlockReason::DeviceDisabled:
            return L"Unavailable - the device is disabled or has a device problem";
        case PrivacyBlockReason::DeviceUnavailable:
            return L"Unavailable - no usable default device was found";
        case PrivacyBlockReason::CameraHardwareOcclusion:
            return L"Likely blocked - the camera driver reports CameraHardware occlusion\n"
                   L"Evidence: advisory while the camera is idle; check its physical control";
        case PrivacyBlockReason::NotInstalled:
            return L"Not installed";
        case PrivacyBlockReason::TaskbarSettingOff:
            return L"Disabled in Windows taskbar settings";
        default:
            return L"";
    }
}

static PCWSTR GetSettingsHint(PrivacyItemKind kind) {
    switch (kind) {
        case PrivacyItemKind::Location:   return L"Click to open Location privacy settings";
        case PrivacyItemKind::Microphone: return L"Click to open Microphone or input settings";
        case PrivacyItemKind::Camera:     return L"Click to open Camera or camera privacy settings";
        case PrivacyItemKind::Copilot:    return L"Click to open the relevant Windows settings";
    }
    return L"";
}

static void SetIconTooltip(FrameworkElement const& fe, PCWSTR label, bool active,
                           PrivacyBlockReason reason, PrivacyItemKind kind,
                           PCWSTR idleLabel = L"Not requested") {
    if (!fe) return;
    std::wstring state;
    if (reason == PrivacyBlockReason::CameraHardwareOcclusion && active) {
        state = L"Blocked - the camera driver reports CameraHardware occlusion\n"
                L"Evidence: reported while Windows shows the camera in use";
    } else if (reason != PrivacyBlockReason::None) {
        state = DescribeBlockReason(reason);
        if (active)
            state += L"\nActivity: Windows also reports this feature in use";
    } else {
        state = active ? L"In use" : idleLabel;
    }
    std::wstring tooltip = label;
    tooltip += L":\n";
    tooltip += state;
    tooltip += L"\n\n";
    tooltip += GetSettingsHint(kind);
    ToolTipService::SetToolTip(
        fe, winrt::box_value(winrt::hstring(tooltip)));
    std::wstring automationName = label;
    automationName += L": ";
    automationName += state;
    winrt::Windows::UI::Xaml::Automation::AutomationProperties::SetName(
        fe, winrt::hstring(automationName));
}

static void UpdateSyntheticTooltips() {
    SetIconTooltip(g_locSlot ? g_locSlot : g_locIcon,
        L"Location", g_locActive.load() || g_locUsage.load(),
        g_locBlockReason.load(), PrivacyItemKind::Location);
    SetIconTooltip(g_micSlot ? g_micSlot : g_micIcon,
        L"Microphone", g_micActive.load() || g_micUsage.load(),
        g_micBlockReason.load(), PrivacyItemKind::Microphone);
    SetIconTooltip(g_camSlot ? g_camSlot : g_camIcon,
        L"Camera", g_camActive.load() || g_camUsage.load(),
        g_camBlockReason.load(), PrivacyItemKind::Camera);
    SetIconTooltip(g_copilotSlot ? g_copilotSlot : g_copilotIcon,
        L"Copilot", g_copilotActive.load(), g_copilotBlockReason.load(),
        PrivacyItemKind::Copilot, L"Installed (not running)");
}

static void UpdateSyntheticState() {
    UpdateSyntheticOpacity();
    UpdateSyntheticTooltips();
}

static PrivacyBlockReason GetBlockReason(PrivacyItemKind kind) {
    switch (kind) {
        case PrivacyItemKind::Location:   return g_locBlockReason.load();
        case PrivacyItemKind::Microphone: return g_micBlockReason.load();
        case PrivacyItemKind::Camera:     return g_camBlockReason.load();
        case PrivacyItemKind::Copilot:    return g_copilotBlockReason.load();
    }
    return PrivacyBlockReason::None;
}

static PCWSTR GetSettingsUri(PrivacyItemKind kind) {
    PrivacyBlockReason reason = GetBlockReason(kind);
    switch (kind) {
        case PrivacyItemKind::Location:
            return L"ms-settings:privacy-location";
        case PrivacyItemKind::Microphone:
            if (reason == PrivacyBlockReason::EndpointMuted ||
                reason == PrivacyBlockReason::DeviceDisabled ||
                reason == PrivacyBlockReason::DeviceUnavailable)
                return L"ms-settings:sound-defaultinputproperties";
            return L"ms-settings:privacy-microphone";
        case PrivacyItemKind::Camera:
            if (reason == PrivacyBlockReason::CameraHardwareOcclusion ||
                reason == PrivacyBlockReason::DeviceDisabled ||
                reason == PrivacyBlockReason::DeviceUnavailable)
                return L"ms-settings:camera";
            return L"ms-settings:privacy-webcam";
        case PrivacyItemKind::Copilot:
            if (reason == PrivacyBlockReason::TaskbarSettingOff)
                return L"ms-settings:taskbar";
            return L"ms-settings:appsfeatures";
    }
    return L"ms-settings:privacy";
}

static void OpenSettingsForItem(PrivacyItemKind kind) {
    PCWSTR uri = GetSettingsUri(kind);
    auto result = reinterpret_cast<INT_PTR>(ShellExecuteW(
        g_taskbarWnd, L"open", uri, nullptr, nullptr, SW_SHOWNORMAL));
    if (result <= 32)
        Wh_Log(L"[Action] Failed to open %s result=%Id", uri, result);
    else
        Wh_Log(L"[Action] Opened %s", uri);
}

static void SetPrivacyActive(PrivacyState::Type type, bool active) {
    switch (type) {
        case PrivacyState::Type::Location: g_locActive.store(active); break;
        case PrivacyState::Type::Mic:      g_micActive.store(active); break;
        case PrivacyState::Type::Camera:   g_camActive.store(active); break;
        case PrivacyState::Type::Both:
            g_locActive.store(active);
            g_micActive.store(active);
            break;
    }
    UpdateSyntheticState();
}

// ============================================================
// Hardware-disabled detection (called from background thread, COM must be initialized)
// ============================================================

// Returns the strongest reason currently reported for the default microphone.
// Endpoint mute is intentionally not called "hardware disabled": laptop Fn
// keys commonly toggle this software/firmware-visible audio endpoint state.
static PrivacyBlockReason CheckMicBlockReason() {
    try {
        using namespace winrt::Windows::Devices::Enumeration;
        auto access = DeviceAccessInformation::CreateFromDeviceClass(
            DeviceClass::AudioCapture).CurrentStatus();
        Wh_Log(L"[Mic] DeviceAccessStatus=%d", static_cast<int>(access));
        if (access == DeviceAccessStatus::DeniedByUser)
            return PrivacyBlockReason::UserAccessDenied;
        if (access == DeviceAccessStatus::DeniedBySystem)
            return PrivacyBlockReason::SystemAccessDenied;
    } catch (...) {
        Wh_Log(L"[Mic] DeviceAccessInformation threw");
    }

    static const struct { HKEY hive; PCWSTR root; PrivacyBlockReason reason; } kConsentChecks[] = {
        {HKEY_CURRENT_USER,
         L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
         L"\\ConsentStore\\microphone", PrivacyBlockReason::UserAccessDenied},
        {HKEY_LOCAL_MACHINE,
         L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
         L"\\ConsentStore\\microphone", PrivacyBlockReason::SystemAccessDenied},
    };
    for (auto const& check : kConsentChecks) {
        HKEY key = nullptr;
        if (RegOpenKeyExW(check.hive, check.root, 0, KEY_READ, &key) != ERROR_SUCCESS)
            continue;
        wchar_t value[64] = {};
        DWORD size = sizeof(value), type = 0;
        LONG result = RegQueryValueExW(
            key, L"Value", nullptr, &type,
            reinterpret_cast<BYTE*>(value), &size);
        RegCloseKey(key);
        if (result == ERROR_SUCCESS && type == REG_SZ &&
            _wcsicmp(value, L"Deny") == 0) {
            Wh_Log(L"[Mic] => blocked (ConsentStore)");
            return check.reason;
        }
    }

    IMMDeviceEnumerator* pEnum = nullptr;
    if (FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                IID_PPV_ARGS(&pEnum))))
        return PrivacyBlockReason::None;
    IMMDevice* pDev = nullptr;
    HRESULT hr = pEnum->GetDefaultAudioEndpoint(eCapture, eConsole, &pDev);
    pEnum->Release();
    if (FAILED(hr)) {
        Wh_Log(L"[Mic] => unavailable (no default capture endpoint), hr=0x%08X", hr);
        return PrivacyBlockReason::DeviceUnavailable;
    }

    PrivacyBlockReason reason = PrivacyBlockReason::None;
    DWORD state = 0;
    if (SUCCEEDED(pDev->GetState(&state))) {
        if (state == DEVICE_STATE_DISABLED)
            reason = PrivacyBlockReason::DeviceDisabled;
        else if (state == DEVICE_STATE_NOTPRESENT || state == DEVICE_STATE_UNPLUGGED)
            reason = PrivacyBlockReason::DeviceUnavailable;
    }
    if (reason == PrivacyBlockReason::None) {
        // Check endpoint master mute (Fn-key path on many laptops)
        IAudioEndpointVolume* pVol = nullptr;
        if (SUCCEEDED(pDev->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                     reinterpret_cast<void**>(&pVol)))) {
            BOOL muted = FALSE;
            if (SUCCEEDED(pVol->GetMute(&muted)) && muted)
                reason = PrivacyBlockReason::EndpointMuted;
            pVol->Release();
        }
    }
    pDev->Release();
    return reason;
}

class MicPrivacyMonitor final : public IMMNotificationClient,
                                public IAudioEndpointVolumeCallback {
public:
    HRESULT Init() {
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      IID_PPV_ARGS(&m_enum));
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] MMDeviceEnumerator failed hr=0x%08X", hr);
            return hr;
        }

        hr = m_enum->RegisterEndpointNotificationCallback(this);
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] RegisterEndpointNotificationCallback failed hr=0x%08X", hr);
        }

        AttachDefaultEndpoint();
        SignalRefresh(L"init");
        return S_OK;
    }

    void Cleanup() {
        DetachEndpointVolume();
        if (m_enum) {
            m_enum->UnregisterEndpointNotificationCallback(this);
            m_enum->Release();
            m_enum = nullptr;
        }
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient)) {
            *ppvObject = static_cast<IMMNotificationClient*>(this);
        } else if (riid == __uuidof(IAudioEndpointVolumeCallback)) {
            *ppvObject = static_cast<IAudioEndpointVolumeCallback*>(this);
        } else {
            *ppvObject = nullptr;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return (ULONG)InterlockedIncrement(&m_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        ULONG count = (ULONG)InterlockedDecrement(&m_refCount);
        if (count == 0) m_refCount = 1; // lifetime is owned by the monitor thread
        return count;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override {
        Wh_Log(L"[MicMon] DeviceStateChanged state=0x%X id=%s", dwNewState, pwstrDeviceId ? pwstrDeviceId : L"");
        AttachDefaultEndpoint();
        SignalRefresh(L"device state");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) override {
        Wh_Log(L"[MicMon] DeviceAdded id=%s", pwstrDeviceId ? pwstrDeviceId : L"");
        AttachDefaultEndpoint();
        SignalRefresh(L"device added");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) override {
        Wh_Log(L"[MicMon] DeviceRemoved id=%s", pwstrDeviceId ? pwstrDeviceId : L"");
        AttachDefaultEndpoint();
        SignalRefresh(L"device removed");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override {
        if (flow == eCapture && (role == eConsole || role == eCommunications)) {
            Wh_Log(L"[MicMon] DefaultDeviceChanged role=%d id=%s", (int)role,
                   pwstrDefaultDeviceId ? pwstrDefaultDeviceId : L"");
            AttachDefaultEndpoint();
            SignalRefresh(L"default device");
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY) override {
        Wh_Log(L"[MicMon] PropertyValueChanged id=%s", pwstrDeviceId ? pwstrDeviceId : L"");
        SignalRefresh(L"property");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify) override {
        if (pNotify) {
            Wh_Log(L"[MicMon] EndpointVolume muted=%d master=%.3f",
                   pNotify->bMuted ? 1 : 0, pNotify->fMasterVolume);
        } else {
            Wh_Log(L"[MicMon] EndpointVolume changed");
        }
        SignalRefresh(L"endpoint volume");
        return S_OK;
    }

private:
    void SignalRefresh(PCWSTR reason) {
        Wh_Log(L"[MicMon] Refresh requested: %s", reason);
        RequestStateRefresh(RefreshMicrophoneState);
    }

    void DetachEndpointVolume() {
        if (m_volume) {
            m_volume->UnregisterControlChangeNotify(this);
            m_volume->Release();
            m_volume = nullptr;
        }
        if (m_device) {
            m_device->Release();
            m_device = nullptr;
        }
    }

    void AttachDefaultEndpoint() {
        if (!m_enum) return;
        DetachEndpointVolume();

        HRESULT hr = m_enum->GetDefaultAudioEndpoint(eCapture, eConsole, &m_device);
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] Default capture endpoint unavailable hr=0x%08X", hr);
            return;
        }

        hr = m_device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, nullptr,
                                reinterpret_cast<void**>(&m_volume));
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] EndpointVolume activate failed hr=0x%08X", hr);
            return;
        }

        hr = m_volume->RegisterControlChangeNotify(this);
        if (FAILED(hr)) {
            Wh_Log(L"[MicMon] RegisterControlChangeNotify failed hr=0x%08X", hr);
        } else {
            Wh_Log(L"[MicMon] Watching default capture endpoint volume");
        }
    }

    volatile LONG m_refCount = 1;
    IMMDeviceEnumerator* m_enum = nullptr;
    IMMDevice* m_device = nullptr;
    IAudioEndpointVolume* m_volume = nullptr;
};

// Portable Windows 11 camera privacy monitor. CameraOcclusionInfo is backed by
// the standard UVC/AVStream privacy control when a camera driver implements it,
// so it covers compliant shutters and kill switches without an OEM-specific
// WMI contract. The MediaCapture is initialized SharedReadOnly and never starts
// a preview/frame reader. The initial open/closed state and persistent
// StateChanged path are live-confirmed on the Legion without creating a webcam
// usage record of their own. A five-minute GetState watchdog detects a stale
// controller without returning to the legacy three-second camera polling.
class CameraPrivacyMonitor {
public:
    HRESULT Init() {
        if (!IsRequested()) {
            Wh_Log(L"[CamMon] Hardware detection not requested");
            return S_FALSE;
        }
        m_wasRequested = true;
        ResetRetrySchedule();
        return TryInitialize();
    }

    void Refresh() {
        bool requested = IsRequested();
        if (!requested) {
            if (m_wasRequested || m_occlusion || m_capture) {
                Wh_Log(L"[CamMon] Releasing camera controller");
                Cleanup();
            }
            m_wasRequested = false;
            m_apiUnsupported = false;
            ResetRetrySchedule();
            return;
        }

        // A settings change can enable monitoring without reloading the mod.
        if (!m_wasRequested) {
            m_wasRequested = true;
            m_apiUnsupported = false;
            ResetRetrySchedule();
            TryInitialize();
            return;
        }

        if (!m_occlusion) {
            if (m_apiUnsupported)
                return;
            ULONGLONG now = GetTickCount64();
            if (!m_nextInitAttempt || now >= m_nextInitAttempt)
                TryInitialize();
            return;
        }

        ULONGLONG now = GetTickCount64();
        if (now - m_lastStateCheck < kWatchdogIntervalMs)
            return;

        // StateChanged is the primary path. This deliberately infrequent read
        // only verifies that the long-lived controller is still responsive.
        m_lastStateCheck = now;
        try {
            UpdateState(m_occlusion.GetState(), L"watchdog");
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"[CamMon] Watchdog failed hr=0x%08X; scheduling retry",
                   static_cast<unsigned>(e.code().value));
            Cleanup();
            ScheduleRetry();
            RequestStateRefresh(RefreshCameraState);
        } catch (...) {
            Wh_Log(L"[CamMon] Watchdog failed; scheduling retry");
            Cleanup();
            ScheduleRetry();
            RequestStateRefresh(RefreshCameraState);
        }
    }

    DWORD NextActionDelayMs() const {
        if (!IsRequested())
            return INFINITE;
        if (!m_wasRequested)
            return 0;
        if (!m_occlusion) {
            if (m_apiUnsupported)
                return INFINITE;
            return DelayUntil(m_nextInitAttempt);
        }
        return DelayUntil(m_lastStateCheck + kWatchdogIntervalMs);
    }

    void ResetFailedRetry() {
        if (!m_occlusion && !m_apiUnsupported)
            ResetRetrySchedule();
    }

    void Cleanup() {
        if (m_occlusion && m_hasStateChangedToken) {
            try {
                m_occlusion.StateChanged(m_stateChangedToken);
            } catch (...) {
                // The camera can disappear before shutdown completes. The
                // monitor is already stopping, so a failed revoke is harmless.
            }
            m_hasStateChangedToken = false;
        }
        m_occlusion = nullptr;
        if (m_capture) {
            try { m_capture.Close(); } catch (...) {}
            m_capture = nullptr;
        }
        g_cameraOcclusionSupported.store(false);
        g_cameraHardwareOccluded.store(false);
    }

private:
    static constexpr ULONGLONG kWatchdogIntervalMs = 5 * 60 * 1000;

    static DWORD DelayUntil(ULONGLONG deadline) {
        if (!deadline)
            return 0;
        ULONGLONG now = GetTickCount64();
        if (now >= deadline)
            return 0;
        return static_cast<DWORD>(std::min<ULONGLONG>(
            deadline - now, static_cast<ULONGLONG>(INFINITE - 1)));
    }

    static bool IsRequested() {
        return g_cameraHardwareDetectionEnabled.load() &&
               g_cameraItemEnabled.load();
    }

    void ResetRetrySchedule() {
        m_retryIndex = 0;
        m_nextInitAttempt = 0;
    }

    void ScheduleRetry() {
        static constexpr ULONGLONG kRetryDelaysMs[] = {
            10 * 1000,
            30 * 1000,
            2 * 60 * 1000,
            10 * 60 * 1000,
            30 * 60 * 1000,
        };
        size_t index = std::min(
            m_retryIndex, ARRAYSIZE(kRetryDelaysMs) - 1);
        ULONGLONG delay = kRetryDelaysMs[index];
        if (m_retryIndex < ARRAYSIZE(kRetryDelaysMs) - 1)
            ++m_retryIndex;
        m_nextInitAttempt = GetTickCount64() + delay;
        Wh_Log(L"[CamMon] Next initialization attempt in %llu ms", delay);
    }

    HRESULT TryInitialize() {
        m_nextInitAttempt = 0;
        Cleanup();

        try {
            using namespace winrt::Windows::Media::Capture;
            using namespace winrt::Windows::Media::Devices;

            MediaCaptureInitializationSettings settings;
            settings.StreamingCaptureMode(StreamingCaptureMode::Video);
            settings.SharingMode(MediaCaptureSharingMode::SharedReadOnly);

            MediaCapture capture;
            capture.InitializeAsync(settings).get();
            CameraOcclusionInfo occlusion =
                capture.VideoDeviceController().CameraOcclusionInfo();
            if (!occlusion || !occlusion.IsOcclusionKindSupported(
                    CameraOcclusionKind::CameraHardware)) {
                Wh_Log(L"[CamMon] CameraHardware occlusion is unsupported");
                m_apiUnsupported = true;
                try { capture.Close(); } catch (...) {}
                return S_FALSE;
            }

            m_apiUnsupported = false;
            m_capture = std::move(capture);
            m_occlusion = std::move(occlusion);
            m_stateChangedToken = m_occlusion.StateChanged(
                [](CameraOcclusionInfo const&,
                   CameraOcclusionStateChangedEventArgs const& args) {
                    UpdateState(args.State(), L"event");
                });
            m_hasStateChangedToken = true;
            UpdateState(m_occlusion.GetState(), L"initial");
            m_lastStateCheck = GetTickCount64();
            ResetRetrySchedule();
            Wh_Log(L"[CamMon] Watching CameraHardware occlusion state");
            return S_OK;
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"[CamMon] Initialize failed hr=0x%08X: %s",
                   static_cast<unsigned>(e.code().value), e.message().c_str());
            Cleanup();
            ScheduleRetry();
            return e.code();
        } catch (...) {
            Wh_Log(L"[CamMon] Initialize failed with an unknown exception");
            Cleanup();
            ScheduleRetry();
            return E_FAIL;
        }
    }

    static void UpdateState(
        winrt::Windows::Media::Devices::CameraOcclusionState const& state,
        PCWSTR reason) {
        using winrt::Windows::Media::Devices::CameraOcclusionKind;
        bool occluded = state.IsOccluded() &&
            state.IsOcclusionKind(CameraOcclusionKind::CameraHardware);
        bool previous = g_cameraHardwareOccluded.exchange(occluded);
        g_cameraOcclusionSupported.store(true);
        Wh_Log(L"[CamMon] %s: hardwareOccluded=%d",
               reason, occluded ? 1 : 0);
        if (previous != occluded)
            RequestStateRefresh(RefreshCameraState);
    }

    ULONGLONG m_lastStateCheck = 0;
    ULONGLONG m_nextInitAttempt = 0;
    size_t m_retryIndex = 0;
    winrt::Windows::Media::Capture::MediaCapture m_capture{nullptr};
    winrt::Windows::Media::Devices::CameraOcclusionInfo m_occlusion{nullptr};
    winrt::event_token m_stateChangedToken{};
    bool m_hasStateChangedToken = false;
    bool m_apiUnsupported = false;
    bool m_wasRequested = false;
};

class RegistryChangeMonitor {
public:
    void AddWatch(HKEY hive, PCWSTR path, DWORD refreshFlags, PCWSTR label) {
        Entry entry;
        entry.hive = hive;
        entry.path = path;
        entry.refreshFlags = refreshFlags;
        entry.label = label;
        m_entries.push_back(std::move(entry));
    }

    DWORD RefreshRegistrations() {
        DWORD refreshFlags = RefreshNone;
        for (auto& entry : m_entries) {
            bool wasArmed = entry.armed;
            TryRegister(entry);
            if (!wasArmed && entry.armed)
                refreshFlags |= entry.refreshFlags;
        }
        return refreshFlags;
    }

    void ResetFailedRetries() {
        for (auto& entry : m_entries) {
            if (!entry.armed && !entry.permanentFailure)
                entry.nextAttempt = 0;
        }
    }

    void AppendWaitHandles(std::vector<HANDLE>& handles) const {
        for (auto const& entry : m_entries) {
            if (entry.armed && entry.event)
                handles.push_back(entry.event);
        }
    }

    DWORD HandleSignaled(HANDLE event) {
        for (auto& entry : m_entries) {
            if (!entry.armed || entry.event != event)
                continue;
            ResetEvent(entry.event);
            Wh_Log(L"[RegMon] Change: %s", entry.label.c_str());
            // Reopen after every notification. This lets a parent fallback
            // move onto the exact key as soon as that key is created, and it
            // also recovers cleanly when the watched key was deleted.
            CloseKey(entry);
            TryRegister(entry);
            return entry.refreshFlags;
        }
        return RefreshNone;
    }

    DWORD NextActionDelayMs() const {
        DWORD result = INFINITE;
        ULONGLONG now = GetTickCount64();
        for (auto const& entry : m_entries) {
            if (entry.armed || entry.permanentFailure)
                continue;
            if (!entry.nextAttempt || now >= entry.nextAttempt)
                return 0;
            result = std::min(result, static_cast<DWORD>(
                std::min<ULONGLONG>(entry.nextAttempt - now,
                                    static_cast<ULONGLONG>(INFINITE - 1))));
        }
        return result;
    }

    void Cleanup() {
        for (auto& entry : m_entries) {
            entry.armed = false;
            if (entry.key) {
                RegCloseKey(entry.key);
                entry.key = nullptr;
            }
            if (entry.event) {
                CloseHandle(entry.event);
                entry.event = nullptr;
            }
        }
        m_entries.clear();
    }

private:
    struct Entry {
        HKEY hive = nullptr;
        std::wstring path;
        std::wstring label;
        DWORD refreshFlags = RefreshNone;
        HKEY key = nullptr;
        HANDLE event = nullptr;
        ULONGLONG nextAttempt = 0;
        size_t retryIndex = 0;
        bool armed = false;
        bool permanentFailure = false;
        bool usingParent = false;
    };

    static void CloseKey(Entry& entry) {
        if (entry.key) {
            RegCloseKey(entry.key);
            entry.key = nullptr;
        }
        entry.armed = false;
        entry.usingParent = false;
    }

    static void ScheduleRetry(Entry& entry, LONG error) {
        CloseKey(entry);
        if (error == ERROR_ACCESS_DENIED) {
            entry.permanentFailure = true;
            Wh_Log(L"[RegMon] Disabled for session: %s error=%d",
                   entry.label.c_str(), error);
            return;
        }

        static constexpr ULONGLONG kRetryDelaysMs[] = {
            10 * 1000,
            30 * 1000,
            2 * 60 * 1000,
            10 * 60 * 1000,
            30 * 60 * 1000,
        };
        size_t index = std::min(
            entry.retryIndex, ARRAYSIZE(kRetryDelaysMs) - 1);
        ULONGLONG delay = kRetryDelaysMs[index];
        if (entry.retryIndex < ARRAYSIZE(kRetryDelaysMs) - 1)
            ++entry.retryIndex;
        entry.nextAttempt = GetTickCount64() + delay;
        Wh_Log(L"[RegMon] Registration failed: %s error=%d retry=%llu ms",
               entry.label.c_str(), error, delay);
    }

    static void TryRegister(Entry& entry) {
        if (entry.armed || entry.permanentFailure)
            return;
        ULONGLONG now = GetTickCount64();
        if (entry.nextAttempt && now < entry.nextAttempt)
            return;

        if (!entry.event) {
            entry.event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            if (!entry.event) {
                ScheduleRetry(entry, static_cast<LONG>(GetLastError()));
                return;
            }
        }

        if (!entry.key) {
            LONG openResult = RegOpenKeyExW(
                entry.hive, entry.path.c_str(), 0, KEY_NOTIFY, &entry.key);
            if (openResult == ERROR_FILE_NOT_FOUND ||
                openResult == ERROR_PATH_NOT_FOUND) {
                std::wstring parent = entry.path;
                size_t separator = parent.find_last_of(L'\\');
                if (separator != std::wstring::npos) {
                    parent.resize(separator);
                    openResult = RegOpenKeyExW(
                        entry.hive, parent.c_str(), 0, KEY_NOTIFY, &entry.key);
                    entry.usingParent = openResult == ERROR_SUCCESS;
                }
            }
            if (openResult != ERROR_SUCCESS) {
                ScheduleRetry(entry, openResult);
                return;
            }
        }

        ResetEvent(entry.event);
        constexpr DWORD kThreadAgnostic = 0x10000000;
        LONG notifyResult = RegNotifyChangeKeyValue(
            entry.key, TRUE,
            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET |
                kThreadAgnostic,
            entry.event, TRUE);
        if (notifyResult != ERROR_SUCCESS) {
            ScheduleRetry(entry, notifyResult);
            return;
        }

        entry.armed = true;
        entry.nextAttempt = 0;
        entry.retryIndex = 0;
        Wh_Log(L"[RegMon] Watching %s%s", entry.label.c_str(),
               entry.usingParent ? L" (parent until key exists)" : L"");
    }

    std::vector<Entry> m_entries;
};

class DeviceStateMonitor {
public:
    void Init() {
        using namespace winrt::Windows::Devices::Enumeration;

        try {
            m_micAccess = DeviceAccessInformation::CreateFromDeviceClass(
                DeviceClass::AudioCapture);
            m_micAccessToken = m_micAccess.AccessChanged(
                [](DeviceAccessInformation const&,
                   DeviceAccessChangedEventArgs const&) {
                    if (!g_unloading)
                        RequestStateRefresh(RefreshMicrophoneState);
                });
            m_hasMicAccessToken = true;
            Wh_Log(L"[DeviceMon] Watching microphone access");
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"[DeviceMon] Microphone access subscription failed hr=0x%08X",
                   static_cast<unsigned>(e.code().value));
        } catch (...) {
            Wh_Log(L"[DeviceMon] Microphone access subscription failed");
        }

        try {
            static const winrt::guid kCameraClass{
                0xca3e7ab9, 0xb4c3, 0x4ae6,
                {0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f}};
            m_cameraAccess = DeviceAccessInformation::CreateFromDeviceClassId(
                kCameraClass);
            m_cameraAccessToken = m_cameraAccess.AccessChanged(
                [](DeviceAccessInformation const&,
                   DeviceAccessChangedEventArgs const&) {
                    if (!g_unloading)
                        RequestStateRefresh(RefreshCameraState);
                });
            m_hasCameraAccessToken = true;
            Wh_Log(L"[DeviceMon] Watching camera access");
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"[DeviceMon] Camera access subscription failed hr=0x%08X",
                   static_cast<unsigned>(e.code().value));
        } catch (...) {
            Wh_Log(L"[DeviceMon] Camera access subscription failed");
        }

        try {
            m_cameraWatcher = DeviceInformation::CreateWatcher(
                DeviceClass::VideoCapture);
            m_cameraAddedToken = m_cameraWatcher.Added(
                [](DeviceWatcher const&, DeviceInformation const&) {
                    if (!g_unloading)
                        RequestStateRefresh(RefreshCameraState);
                });
            m_cameraRemovedToken = m_cameraWatcher.Removed(
                [](DeviceWatcher const&, DeviceInformationUpdate const&) {
                    if (!g_unloading)
                        RequestStateRefresh(RefreshCameraState);
                });
            m_cameraUpdatedToken = m_cameraWatcher.Updated(
                [](DeviceWatcher const&, DeviceInformationUpdate const&) {
                    if (!g_unloading)
                        RequestStateRefresh(RefreshCameraState);
                });
            m_hasCameraWatcherTokens = true;
            m_cameraWatcher.Start();
            Wh_Log(L"[DeviceMon] Watching camera device topology");
        } catch (winrt::hresult_error const& e) {
            Wh_Log(L"[DeviceMon] Camera watcher failed hr=0x%08X",
                   static_cast<unsigned>(e.code().value));
            CleanupCameraWatcher();
        } catch (...) {
            Wh_Log(L"[DeviceMon] Camera watcher failed");
            CleanupCameraWatcher();
        }
    }

    void Cleanup() {
        CleanupCameraWatcher();
        if (m_micAccess && m_hasMicAccessToken) {
            try { m_micAccess.AccessChanged(m_micAccessToken); } catch (...) {}
        }
        if (m_cameraAccess && m_hasCameraAccessToken) {
            try { m_cameraAccess.AccessChanged(m_cameraAccessToken); } catch (...) {}
        }
        m_hasMicAccessToken = false;
        m_hasCameraAccessToken = false;
        m_micAccess = nullptr;
        m_cameraAccess = nullptr;
    }

private:
    void CleanupCameraWatcher() {
        if (!m_cameraWatcher)
            return;
        if (m_hasCameraWatcherTokens) {
            try { m_cameraWatcher.Added(m_cameraAddedToken); } catch (...) {}
            try { m_cameraWatcher.Removed(m_cameraRemovedToken); } catch (...) {}
            try { m_cameraWatcher.Updated(m_cameraUpdatedToken); } catch (...) {}
        }
        m_hasCameraWatcherTokens = false;
        try { m_cameraWatcher.Stop(); } catch (...) {}
        m_cameraWatcher = nullptr;
    }

    winrt::Windows::Devices::Enumeration::DeviceAccessInformation
        m_micAccess{nullptr};
    winrt::Windows::Devices::Enumeration::DeviceAccessInformation
        m_cameraAccess{nullptr};
    winrt::Windows::Devices::Enumeration::DeviceWatcher
        m_cameraWatcher{nullptr};
    winrt::event_token m_micAccessToken{};
    winrt::event_token m_cameraAccessToken{};
    winrt::event_token m_cameraAddedToken{};
    winrt::event_token m_cameraRemovedToken{};
    winrt::event_token m_cameraUpdatedToken{};
    bool m_hasMicAccessToken = false;
    bool m_hasCameraAccessToken = false;
    bool m_hasCameraWatcherTokens = false;
};

// Returns the strongest observed camera block reason. CameraHardware occlusion
// is a driver report, not proof that pixels cannot be captured; Microsoft marks
// it advisory when the camera is not actively streaming.
static PrivacyBlockReason CheckCameraBlockReason() {
    // Check 0: Windows 11 standard camera shutter/kill-switch state.
    if (g_cameraOcclusionSupported.load() &&
        g_cameraHardwareOccluded.load()) {
        Wh_Log(L"[Cam] => likely blocked (CameraHardware occlusion report)");
        return PrivacyBlockReason::CameraHardwareOcclusion;
    }
    // Check 1: WinRT DeviceAccessInformation (most reliable for Privacy Settings toggle)
    try {
        using namespace winrt::Windows::Devices::Enumeration;
        static const winrt::guid kCameraClass{0xca3e7ab9, 0xb4c3, 0x4ae6,
            {0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f}};
        auto info = DeviceAccessInformation::CreateFromDeviceClassId(kCameraClass);
        auto status = info.CurrentStatus();
        Wh_Log(L"[Cam] DeviceAccessStatus=%d (1=DeniedByUser,2=DeniedBySystem)", (int)status);
        if (status == DeviceAccessStatus::DeniedByUser ||
            status == DeviceAccessStatus::DeniedBySystem) {
            Wh_Log(L"[Cam] => blocked (DeviceAccess consent)");
            return status == DeviceAccessStatus::DeniedByUser
                ? PrivacyBlockReason::UserAccessDenied
                : PrivacyBlockReason::SystemAccessDenied;
        }
    } catch (...) {
        Wh_Log(L"[Cam] DeviceAccessInformation threw");
    }

    static const GUID GUID_DEVCLASS_CAMERA_LOCAL =
        {0xca3e7ab9, 0xb4c3, 0x4ae6, {0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f}};
    // Check if any camera device is registered in the system at all
    HDEVINFO allDevs = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA_LOCAL, nullptr, nullptr, 0);
    if (allDevs == INVALID_HANDLE_VALUE) {
        Wh_Log(L"[Cam] allDevs INVALID_HANDLE_VALUE err=%u", GetLastError());
        return PrivacyBlockReason::None;
    }
    SP_DEVINFO_DATA d{}; d.cbSize = sizeof(d);
    bool hasAny = SetupDiEnumDeviceInfo(allDevs, 0, &d) == TRUE;
    SetupDiDestroyDeviceInfoList(allDevs);
    Wh_Log(L"[Cam] hasAny=%d", hasAny);
    if (!hasAny) {
        Wh_Log(L"[Cam] => unavailable (no camera hardware)");
        return PrivacyBlockReason::DeviceUnavailable;
    }
    // Check if any non-IR camera is present (powered on)
    // Filter out IR/Hello cameras which are always-on and would mask a hardware kill switch.
    HDEVINFO presentDevs = SetupDiGetClassDevs(&GUID_DEVCLASS_CAMERA_LOCAL, nullptr, nullptr, DIGCF_PRESENT);
    bool hasPresent = false;
    if (presentDevs != INVALID_HANDLE_VALUE) {
        SP_DEVINFO_DATA pd{}; pd.cbSize = sizeof(pd);
        for (DWORD idx = 0; !hasPresent && SetupDiEnumDeviceInfo(presentDevs, idx, &pd); idx++) {
            wchar_t name[256] = {}; DWORD type = 0, sz = sizeof(name);
            SetupDiGetDeviceRegistryPropertyW(presentDevs, &pd, SPDRP_FRIENDLYNAME,
                                              &type, (BYTE*)name, sz, nullptr);
            std::wstring_view nm{name};
            bool isIR = nm.find(L"IR") != std::wstring_view::npos ||
                        (nm.find(L"Hello") != std::wstring_view::npos) ||
                        (nm.find(L"Face")  != std::wstring_view::npos);
            if (!isIR) {
                // Also check if the device is disabled in Device Manager.
                // Hardware kill switches sometimes disable the device rather than
                // removing it from DIGCF_PRESENT entirely.
                ULONG devStatus = 0, devProblem = 0;
                bool hasProblem = false;
                if (CM_Get_DevNode_Status(&devStatus, &devProblem, pd.DevInst, 0) == CR_SUCCESS) {
                    hasProblem = (devStatus & DN_HAS_PROBLEM) != 0;
                }
                Wh_Log(L"[Cam] present device: '%s' isIR=%d hasProblem=%d (status=0x%X prob=%u)",
                    name, isIR ? 1 : 0, hasProblem ? 1 : 0, devStatus, devProblem);
                if (!hasProblem) hasPresent = true;
            } else {
                Wh_Log(L"[Cam] present device: '%s' isIR=1 (filtered)", name);
            }
        }
        SetupDiDestroyDeviceInfoList(presentDevs);
    }
    Wh_Log(L"[Cam] hasPresent(non-IR)=%d", hasPresent);
    if (!hasPresent) {
        Wh_Log(L"[Cam] => unavailable/disabled (no usable non-IR camera)");
        return PrivacyBlockReason::DeviceDisabled;
    }
    // Check: per-user consent (Privacy & Security → Camera toggle → HKCU)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\webcam",
                0, KEY_READ, &hk);
        Wh_Log(L"[Cam] HKCU webcam ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Cam] HKCU Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Cam] => blocked (HKCU consent)");
                return PrivacyBlockReason::UserAccessDenied;
            }
        }
    }
    // Check: machine-wide policy (HKLM)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\webcam",
                0, KEY_READ, &hk);
        Wh_Log(L"[Cam] HKLM webcam ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Cam] HKLM Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Cam] => blocked (HKLM consent)");
                return PrivacyBlockReason::SystemAccessDenied;
            }
        }
    }
    Wh_Log(L"[Cam] => enabled");
    return PrivacyBlockReason::None;
}

// Returns true if a Copilot app package is registered in the AppModel repository for the
// current user or machine. The package data directory in %LOCALAPPDATA%\Packages is NOT
// used — that directory survives uninstall (it holds user data) and would give a false
// positive after removal.
//
// Deliberately NOT matched: MicrosoftWindows.Client.WebExperience — that package is the
// Widgets host, present on virtually every Windows 11 install, and counting it as Copilot
// made the mod report "installed" on Copilot-free machines (live-verified 2026-07-17).
static bool CheckCopilotInstalled() {
    // Sub-keys under Repository\Packages are named <PackageFullName> e.g.
    //   Microsoft.Copilot_<ver>_x64__<pub>
    // Stale keys can survive uninstall, so a key only counts if its package path
    // still exists on disk.
    static const wchar_t* const kPrefixes[] = {
        L"Microsoft.Copilot_",
        L"Microsoft.Windows.Ai.Copilot_",
    };
    static const wchar_t* const kRoots[] = {
        L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\AppModel\\Repository\\Packages",                     // per-user (HKCU)
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion"
            L"\\AppModel\\Repository\\Packages",                     // machine-wide (HKLM)
    };
    static const HKEY kHives[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    for (int h = 0; h < 2; h++) {
        HKEY hPkg = nullptr;
        LONG openR = RegOpenKeyExW(kHives[h], kRoots[h], 0,
                                   KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hPkg);
        Wh_Log(L"[Copilot] AppModel %s open=%d", h == 0 ? L"HKCU" : L"HKLM", openR);
        if (openR != ERROR_SUCCESS) continue;
        wchar_t name[256]; DWORD nameLen;
        bool found = false;
        for (DWORD i = 0; !found; i++) {
            nameLen = ARRAYSIZE(name);
            LONG e = RegEnumKeyExW(hPkg, i, name, &nameLen,
                                   nullptr, nullptr, nullptr, nullptr);
            if (e == ERROR_NO_MORE_ITEMS) break;
            if (e != ERROR_SUCCESS)       continue;
            bool prefixMatch = false;
            for (const auto* prefix : kPrefixes) {
                if (wcsncmp(name, prefix, wcslen(prefix)) == 0) {
                    prefixMatch = true;
                    break;
                }
            }
            if (prefixMatch) {
                HKEY hItem = nullptr;
                if (RegOpenKeyExW(hPkg, name, 0, KEY_READ, &hItem) == ERROR_SUCCESS) {
                    wchar_t path[MAX_PATH] = {};
                    DWORD pathLen = sizeof(path), type = 0;
                    LONG pathR = RegQueryValueExW(hItem, L"PackageRootFolder", nullptr,
                                                  &type, (LPBYTE)path, &pathLen);
                    if (pathR != ERROR_SUCCESS) {
                        pathLen = sizeof(path); type = 0;
                        pathR = RegQueryValueExW(hItem, L"Path", nullptr,
                                                 &type, (LPBYTE)path, &pathLen);
                    }
                    RegCloseKey(hItem);
                    bool pathExists = pathR == ERROR_SUCCESS &&
                                      (type == REG_SZ || type == REG_EXPAND_SZ) &&
                                      GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES;
                    Wh_Log(L"[Copilot] package key: %s path='%s' exists=%d",
                           name, path, pathExists ? 1 : 0);
                    if (pathExists) found = true;
                }
            }
        }
        RegCloseKey(hPkg);
        if (found) { Wh_Log(L"[Copilot] => installed"); return true; }
    }
    Wh_Log(L"[Copilot] => not installed");
    return false;
}

// Returns true if a Copilot-related process is currently running.
static bool CheckCopilotActive() {
    static const wchar_t* const exes[] = {
        L"Copilot.exe", L"AIHost.exe", L"copilotwindows.exe", L"Microsoft.Copilot.exe"
    };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    PROCESSENTRY32W pe{}; pe.dwSize = sizeof(pe);
    bool found = false;
    if (Process32FirstW(snap, &pe)) {
        do {
            for (const auto* exe : exes) {
                if (_wcsicmp(pe.szExeFile, exe) == 0) { found = true; break; }
            }
        } while (!found && Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return found;
}

// Returns why Copilot is unavailable. "Installed but not running" is idle,
// not disabled.
static PrivacyBlockReason CheckCopilotBlockReason() {
    // Group policy: TurnOffWindowsCopilot=1 under either hive is a hard disable.
    static const HKEY kPolicyHives[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
    for (HKEY hive : kPolicyHives) {
        HKEY hKey = nullptr;
        if (RegOpenKeyExW(hive,
                L"Software\\Policies\\Microsoft\\Windows\\WindowsCopilot",
                0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD off = 0, cb = sizeof(off);
            LONG r = RegQueryValueExW(hKey, L"TurnOffWindowsCopilot", nullptr, nullptr,
                                      reinterpret_cast<BYTE*>(&off), &cb);
            RegCloseKey(hKey);
            if (r == ERROR_SUCCESS && off != 0) {
                Wh_Log(L"[Copilot] => disabled (TurnOffWindowsCopilot policy)");
                return PrivacyBlockReason::PolicyDisabled;
            }
        }
    }
    // ShowCopilotButton=0 means the user deliberately turned Copilot off in
    // Settings > Personalization > Taskbar. Treat this as the explicit-disable signal,
    // analogous to revoking mic/camera consent.
    DWORD showButton = 1;
    DWORD cbData = sizeof(showButton);
    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueExW(hKey, L"ShowCopilotButton", nullptr, nullptr,
                         reinterpret_cast<BYTE*>(&showButton), &cbData);
        RegCloseKey(hKey);
    }
    Wh_Log(L"[Copilot] ShowCopilotButton=%u  installed=%d", showButton, g_copilotInstalled.load() ? 1 : 0);
    if (!g_copilotInstalled.load())
        return PrivacyBlockReason::NotInstalled;
    if (showButton == 0)
        return PrivacyBlockReason::TaskbarSettingOff;
    return PrivacyBlockReason::None;
}

// Returns the exact Windows layer currently blocking location.
static PrivacyBlockReason CheckLocationBlockReason() {
    // Check 0: Group Policy hard-disable
    {
        HKEY hk = nullptr;
        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Policies\\Microsoft\\Windows\\LocationAndSensors",
                0, KEY_READ, &hk) == ERROR_SUCCESS) {
            DWORD val = 0; DWORD sz = sizeof(val);
            LONG r = RegQueryValueExW(hk, L"DisableLocation", nullptr, nullptr, (LPBYTE)&val, &sz);
            RegCloseKey(hk);
            if (r == ERROR_SUCCESS && val != 0) {
                Wh_Log(L"[Loc] => disabled (Group Policy)");
                return PrivacyBlockReason::PolicyDisabled;
            }
        }
    }
    // Check 1: Geolocation service master switch (lfsvc service configuration)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\lfsvc\\Service\\Configuration",
                0, KEY_READ, &hk);
        Wh_Log(L"[Loc] lfsvc key open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            DWORD status = 0xFFFFFFFF, valLen = sizeof(status);
            LONG qr = RegQueryValueExW(hk, L"Status", nullptr, nullptr, (LPBYTE)&status, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Loc] lfsvc Status: qr=%d val=%u", qr, status);
            if (qr == ERROR_SUCCESS && status == 0) {
                Wh_Log(L"[Loc] => disabled (lfsvc)");
                return PrivacyBlockReason::ServiceDisabled;
            }
        }
    }
    // Check 2: per-user consent (Privacy & Security → Location services toggle → HKCU)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\location",
                0, KEY_READ, &hk);
        Wh_Log(L"[Loc] HKCU ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Loc] HKCU Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Loc] => blocked (HKCU consent)");
                return PrivacyBlockReason::UserAccessDenied;
            }
        }
    }
    // Check 3: machine-wide policy (HKLM)
    {
        HKEY hk = nullptr;
        LONG openR = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
                L"\\ConsentStore\\location",
                0, KEY_READ, &hk);
        Wh_Log(L"[Loc] HKLM ConsentStore open=%d", openR);
        if (openR == ERROR_SUCCESS) {
            wchar_t val[64] = {};
            DWORD valLen = sizeof(val), type = 0;
            LONG r = RegQueryValueExW(hk, L"Value", nullptr, &type, (LPBYTE)val, &valLen);
            RegCloseKey(hk);
            Wh_Log(L"[Loc] HKLM Value: r=%d type=%u val='%s'", r, type, val);
            if (r == ERROR_SUCCESS && type == REG_SZ && _wcsicmp(val, L"Deny") == 0) {
                Wh_Log(L"[Loc] => blocked (HKLM consent)");
                return PrivacyBlockReason::SystemAccessDenied;
            }
        }
    }
    Wh_Log(L"[Loc] => enabled");
    return PrivacyBlockReason::None;
}

// In-use detection via CapabilityAccessManager usage records: each app that
// uses a capability gets a subkey under ConsentStore\<capability> (packaged
// apps directly, win32 apps under NonPackaged\) with LastUsedTimeStart /
// LastUsedTimeStop QWORDs. Stop == 0 while Start is set means the app is using
// the capability RIGHT NOW. This is how Settings > Privacy shows "currently in
// use", and it fires for hardware cameras/mics that never get a tray glyph.
static bool ScanConsentUsage(HKEY hive, const std::wstring& path, int depth = 0) {
    if (depth > 1) return false;
    HKEY hk = nullptr;
    if (RegOpenKeyExW(hive, path.c_str(), 0,
                      KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hk) != ERROR_SUCCESS)
        return false;
    bool inUse = false;
    wchar_t name[256];
    for (DWORD i = 0; !inUse; i++) {
        DWORD nameLen = ARRAYSIZE(name);
        LONG e = RegEnumKeyExW(hk, i, name, &nameLen,
                               nullptr, nullptr, nullptr, nullptr);
        if (e == ERROR_NO_MORE_ITEMS) break;
        if (e != ERROR_SUCCESS) continue;
        if (_wcsicmp(name, L"NonPackaged") == 0) {
            inUse = ScanConsentUsage(hive, path + L"\\" + name, depth + 1);
            continue;
        }
        HKEY hApp = nullptr;
        if (RegOpenKeyExW(hk, name, 0, KEY_READ, &hApp) == ERROR_SUCCESS) {
            ULONGLONG start = 0, stop = 0;
            DWORD sz = sizeof(start);
            bool hasStart = RegQueryValueExW(hApp, L"LastUsedTimeStart", nullptr,
                                nullptr, (LPBYTE)&start, &sz) == ERROR_SUCCESS &&
                            start != 0;
            sz = sizeof(stop);
            bool hasStop = RegQueryValueExW(hApp, L"LastUsedTimeStop", nullptr,
                               nullptr, (LPBYTE)&stop, &sz) == ERROR_SUCCESS;
            RegCloseKey(hApp);
            if (hasStart && hasStop && stop == 0) {
                Wh_Log(L"[Usage] %s in use by %s", path.c_str(), name);
                inUse = true;
            }
        }
    }
    RegCloseKey(hk);
    return inUse;
}

static bool CheckCapabilityInUse(PCWSTR capability) {
    std::wstring base =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\CapabilityAccessManager"
        L"\\ConsentStore\\";
    return ScanConsentUsage(HKEY_CURRENT_USER,  base + capability) ||
           ScanConsentUsage(HKEY_LOCAL_MACHINE, base + capability);
}

// Refresh only the domains whose native notification or sparse watchdog fired.
// Must be called from a thread with COM initialized (COINIT_MULTITHREADED).
static void UpdatePrivacyStates(DWORD flags) {
    if (g_unloading || !flags)
        return;

    bool changed = false;
    if (flags & RefreshLocationState) {
        PrivacyBlockReason reason = CheckLocationBlockReason();
        bool disabled = reason != PrivacyBlockReason::None;
        changed |= g_locBlockReason.exchange(reason) != reason;
        changed |= g_locDisabled.exchange(disabled) != disabled;
    }
    if (flags & RefreshMicrophoneState) {
        PrivacyBlockReason reason = CheckMicBlockReason();
        bool disabled = reason != PrivacyBlockReason::None;
        changed |= g_micBlockReason.exchange(reason) != reason;
        changed |= g_micDisabled.exchange(disabled) != disabled;
    }
    if (flags & RefreshCameraState) {
        PrivacyBlockReason reason = CheckCameraBlockReason();
        bool disabled = reason != PrivacyBlockReason::None;
        changed |= g_camBlockReason.exchange(reason) != reason;
        changed |= g_camDisabled.exchange(disabled) != disabled;
    }
    if (flags & RefreshLocationUsage) {
        bool inUse = CheckCapabilityInUse(L"location");
        changed |= g_locUsage.exchange(inUse) != inUse;
    }
    if (flags & RefreshMicrophoneUsage) {
        bool inUse = CheckCapabilityInUse(L"microphone");
        changed |= g_micUsage.exchange(inUse) != inUse;
    }
    if (flags & RefreshCameraUsage) {
        bool inUse = CheckCapabilityInUse(L"webcam");
        changed |= g_camUsage.exchange(inUse) != inUse;
    }
    if (flags & RefreshCopilotState) {
        bool installed = g_copilotItemEnabled.load() &&
                         CheckCopilotInstalled();
        changed |= g_copilotInstalled.exchange(installed) != installed;
        PrivacyBlockReason reason = g_copilotItemEnabled.load()
            ? CheckCopilotBlockReason()
            : PrivacyBlockReason::NotInstalled;
        bool disabled = reason != PrivacyBlockReason::None;
        changed |= g_copilotBlockReason.exchange(reason) != reason;
        changed |= g_copilotDisabled.exchange(disabled) != disabled;
    }
    if (flags & RefreshCopilotActivity) {
        bool active = g_copilotItemEnabled.load() && CheckCopilotActive();
        changed |= g_copilotActive.exchange(active) != active;
    }

    Wh_Log(L"[Refresh] flags=0x%08X changed=%d loc=%d mic=%d cam=%d copInst=%d copAct=%d copDis=%d",
           flags, changed ? 1 : 0,
           g_locDisabled.load(), g_micDisabled.load(), g_camDisabled.load(),
           g_copilotInstalled.load(), g_copilotActive.load(),
           g_copilotDisabled.load());
    if (changed && !g_unloading && g_taskbarWnd) {
        RunFromWindowThread(g_taskbarWnd, [](void*) {
            if (!g_unloading) UpdateSyntheticState();
        }, nullptr);
    }
}

static TextBlock MakeIconTextBlock(const wchar_t* glyph) {
    TextBlock tb;
    tb.Text(glyph);
    tb.FontFamily(FontFamily(L"Segoe MDL2 Assets"));
    tb.FontSize((double)g_settings.iconSize);
    tb.VerticalAlignment(VerticalAlignment::Center);
    tb.HorizontalAlignment(HorizontalAlignment::Center);
    tb.TextWrapping(TextWrapping::NoWrap);
    return tb;
}

// A real emphasis layer: concentric translucent halos plus optional animation.
// The host keeps the icon's exact layout size; larger children render outside
// that box without participating in taskbar measurement.
static FrameworkElement MakeGlowVisual(winrt::Windows::UI::Color color) {
    using namespace winrt::Windows::UI::Xaml::Media::Animation;
    using winrt::Windows::Foundation::IReference;
    using winrt::Windows::Foundation::TimeSpan;
    using winrt::Windows::UI::Xaml::Shapes::Ellipse;

    Grid host;
    double iconSize = static_cast<double>(g_settings.iconSize);
    double reach = g_settings.glowSize / 100.0;
    double strength = g_settings.glowOpacity / 100.0;
    host.Width(iconSize);
    host.Height(iconSize);
    host.HorizontalAlignment(HorizontalAlignment::Center);
    host.VerticalAlignment(VerticalAlignment::Center);
    host.IsHitTestVisible(false);
    host.Visibility(Visibility::Collapsed);

    auto makeBrush = [color]() {
        SolidColorBrush brush;
        brush.Color(color);
        return brush;
    };
    auto addHalo = [&](double diameter, double opacity) {
        Ellipse halo;
        halo.Width(diameter);
        halo.Height(diameter);
        halo.HorizontalAlignment(HorizontalAlignment::Center);
        halo.VerticalAlignment(VerticalAlignment::Center);
        halo.IsHitTestVisible(false);
        halo.Fill(makeBrush());
        halo.Opacity(std::clamp(opacity, 0.0, 1.0));
        host.Children().Append(halo);
    };

    // Several low-alpha layers read as a bloom without relying on a compositor
    // effect that may be unavailable inside Explorer's taskbar XAML island.
    addHalo(iconSize * reach, strength * 0.08);
    addHalo(iconSize * (1.0 + (reach - 1.0) * 0.55), strength * 0.14);
    addHalo(iconSize * 1.20, strength * 0.24);

    GlowAnimationState animationState;
    animationState.element = host;

    auto makeAnimation = [&](DependencyObject const& target, PCWSTR property,
                             double from, double to, int durationMs,
                             int beginMs = 0) {
        DoubleAnimation animation;
        animation.From(winrt::box_value(from).as<IReference<double>>());
        animation.To(winrt::box_value(to).as<IReference<double>>());
        animation.Duration(DurationHelper::FromTimeSpan(
            TimeSpan{static_cast<int64_t>(durationMs) * 10000}));
        if (beginMs > 0) {
            animation.BeginTime(winrt::box_value(TimeSpan{
                static_cast<int64_t>(beginMs) * 10000}).as<IReference<TimeSpan>>());
        }
        animation.RepeatBehavior(RepeatBehaviorHelper::Forever());
        animation.EnableDependentAnimation(true);
        Storyboard::SetTarget(animation, target);
        Storyboard::SetTargetProperty(animation, property);
        return animation;
    };

    if (g_settings.glowStyle == L"pulse") {
        Storyboard storyboard;
        auto opacity = makeAnimation(host, L"Opacity", 0.28, 1.0,
                                     g_settings.glowSpeed);
        opacity.AutoReverse(true);
        storyboard.Children().Append(opacity);
        animationState.storyboards.push_back(storyboard);
    } else if (g_settings.glowStyle == L"radiate") {
        constexpr int kRingCount = 3;
        for (int i = 0; i < kRingCount; ++i) {
            Ellipse ring;
            ring.Width(iconSize);
            ring.Height(iconSize);
            ring.HorizontalAlignment(HorizontalAlignment::Center);
            ring.VerticalAlignment(VerticalAlignment::Center);
            ring.IsHitTestVisible(false);
            ring.Fill(nullptr);
            ring.Stroke(makeBrush());
            ring.StrokeThickness(std::max(1.0, iconSize * 0.075));
            ring.Opacity(0.0);
            ring.RenderTransformOrigin({0.5f, 0.5f});
            ScaleTransform scale;
            scale.ScaleX(0.72);
            scale.ScaleY(0.72);
            ring.RenderTransform(scale);
            host.Children().Append(ring);

            int phaseMs = g_settings.glowSpeed * i / kRingCount;
            Storyboard storyboard;
            storyboard.Children().Append(makeAnimation(
                scale, L"ScaleX", 0.72, reach, g_settings.glowSpeed, phaseMs));
            storyboard.Children().Append(makeAnimation(
                scale, L"ScaleY", 0.72, reach, g_settings.glowSpeed, phaseMs));
            storyboard.Children().Append(makeAnimation(
                ring, L"Opacity", strength, 0.0,
                g_settings.glowSpeed, phaseMs));
            animationState.storyboards.push_back(storyboard);
        }
    }

    g_glowAnimationStates.push_back(std::move(animationState));
    return host;
}

static void ApplyOffset(FrameworkElement const& fe, int x, int y) {
    if (!fe) return;
    if (x != 0 || y != 0) {
        TranslateTransform tt;
        tt.X((double)x); tt.Y((double)y);
        fe.RenderTransform(tt);
    } else {
        fe.ClearValue(UIElement::RenderTransformProperty());
    }
}

static bool InjectSyntheticIcons(FrameworkElement root) {
    auto gridElem = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!gridElem) { Wh_Log(L"[Inject] SystemTrayFrameGrid not found"); return false; }
    auto gridParent = gridElem.try_as<Grid>();
    if (!gridParent) { Wh_Log(L"[Inject] SystemTrayFrameGrid not a Grid"); return false; }

    // Idempotent check.
    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"PrivacyAnchorBar")
            return true;
    }

    auto activeItems = ParseItemOrder(g_settings.itemOrder);
    if (activeItems.empty()) {
        Wh_Log(L"[Inject] itemOrder has no valid tokens");
        return true;
    }

    auto findNamedDirect = [&](const wchar_t* name) -> FrameworkElement {
        for (auto child : gridParent.Children()) {
            if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == name)
                return fe;
        }
        return nullptr;
    };

    FrameworkElement refElem = nullptr;
    bool insertAfterRef = false;
    if      (g_settings.position == L"beforeOmni")
        refElem = findNamedDirect(L"ControlCenterButton");
    else if (g_settings.position == L"beforeClock")
        refElem = findNamedDirect(L"NotificationCenterButton");
    else if (g_settings.position == L"afterClock")
        refElem = findNamedDirect(L"ShowDesktopStack");
    else if (g_settings.position == L"afterShowDesktop") {
        refElem = findNamedDirect(L"ShowDesktopStack");
        insertAfterRef = true;
    }

    int insertCol = 0;
    if (insertAfterRef && refElem)  insertCol = Grid::GetColumn(refElem) + 1;
    else if (refElem)               insertCol = Grid::GetColumn(refElem);

    ColumnDefinition cd;
    cd.Width({ 1.0, GridUnitType::Auto });
    if ((uint32_t)insertCol < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().InsertAt(insertCol, cd);
    else
        gridParent.ColumnDefinitions().Append(cd);

    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int col  = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (col >= insertCol)             Grid::SetColumn(fe, col + 1);
        else if (col + span > insertCol)  Grid::SetColumnSpan(fe, span + 1);
    }

    // ── Build the anchor bar ────────────────────────────────────
    Grid bar;
    bar.Name(L"PrivacyAnchorBar");
    bar.VerticalAlignment(VerticalAlignment::Center);
    bar.HorizontalAlignment(HorizontalAlignment::Center);
    bar.Margin({ (double)g_settings.groupPaddingLeft, 0.0,
                 (double)g_settings.groupPaddingRight, 0.0 });
    ApplyOffset(bar, g_settings.groupOffsetX, g_settings.groupOffsetY);

    int N = (int)activeItems.size();

    // Grid shape from the smart-grid template; mode derives from the rows /
    // columns settings (0 = auto).
    grid::Config cfg;
    cfg.fillOrder          = g_settings.fillOrder;
    cfg.shortGroupPosition = g_settings.shortGroupPosition;
    cfg.shortGroupAlign    = g_settings.shortGroupAlign;
    cfg.rows               = g_settings.gridRows;
    cfg.columns            = g_settings.gridColumns;
    if      (g_settings.gridRows > 0 && g_settings.gridColumns > 0)
        cfg.mode = grid::GridMode::FixedGrid;
    else if (g_settings.gridRows > 0)
        cfg.mode = grid::GridMode::FixedRows;
    else if (g_settings.gridColumns > 0)
        cfg.mode = grid::GridMode::FixedColumns;
    else
        cfg.mode = grid::GridMode::AutoSmart;

    // Row capacity from the taskbar height and icon pitch: a single column on
    // double-height taskbars, more columns when the stack doesn't fit.
    {
        int taskbarH = 48;
        HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
        RECT rc{};
        if (hWnd && GetWindowRect(hWnd, &rc))
            taskbarH = (int)(rc.bottom - rc.top);
        int pitch = g_settings.iconSize + std::max(0, g_settings.buttonSpacing);
        cfg.availableRows = std::max(1,
            (taskbarH + std::max(0, g_settings.buttonSpacing)) / std::max(1, pitch));
    }

    grid::Layout layout = grid::ComputeLayout(N, cfg);
    int rows = layout.rows;
    int cols = layout.columns;

    for (int r = 0; r < rows; r++) {
        RowDefinition rd;
        rd.Height({ (double)g_settings.iconSize, GridUnitType::Pixel });
        bar.RowDefinitions().Append(rd);
    }
    for (int c = 0; c < cols; c++) {
        ColumnDefinition bcd;
        bcd.Width({ (double)g_settings.iconSize, GridUnitType::Pixel });
        bar.ColumnDefinitions().Append(bcd);
    }
    if (g_settings.buttonSpacing > 0) {
        bar.ColumnSpacing((double)g_settings.buttonSpacing);
        bar.RowSpacing((double)g_settings.buttonSpacing);
    }

    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locSlot = nullptr; g_micSlot = nullptr; g_camSlot = nullptr; g_copilotSlot = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;

    for (int i = 0; i < N; i++) {
        const auto& token = activeItems[i];
        const wchar_t* glyph    = L"";
        const wchar_t* iconFont = L"Segoe MDL2 Assets";
        bool  isActive, isDisabled;
        PrivacyBlockReason blockReason;
        PrivacyItemKind itemKind;
        int   offX, offY;
        const wchar_t* label;
        const wchar_t* idleLabel     = L"Not requested";

        if (token == L"location") {
            glyph        = L"\xE37A";
            isActive     = g_locActive.load() || g_locUsage.load();
            isDisabled   = g_locDisabled.load();
            blockReason  = g_locBlockReason.load();
            itemKind     = PrivacyItemKind::Location;
            offX         = g_settings.locationOffsetX;
            offY         = g_settings.locationOffsetY;
            label        = L"Location";
        } else if (token == L"mic") {
            glyph        = L"\xE720";
            isActive     = g_micActive.load() || g_micUsage.load();
            isDisabled   = g_micDisabled.load();
            blockReason  = g_micBlockReason.load();
            itemKind     = PrivacyItemKind::Microphone;
            offX         = g_settings.micOffsetX;
            offY         = g_settings.micOffsetY;
            label        = L"Microphone";
        } else if (token == L"camera") {
            glyph        = L"\xE722";
            isActive     = g_camActive.load() || g_camUsage.load();
            isDisabled   = g_camDisabled.load();
            blockReason  = g_camBlockReason.load();
            itemKind     = PrivacyItemKind::Camera;
            offX         = g_settings.cameraOffsetX;
            offY         = g_settings.cameraOffsetY;
            label        = L"Camera";
        } else {  // copilot
            isActive      = g_copilotActive.load();
            isDisabled    = g_copilotDisabled.load();
            blockReason   = g_copilotBlockReason.load();
            itemKind      = PrivacyItemKind::Copilot;
            offX          = g_settings.copilotOffsetX;
            offY          = g_settings.copilotOffsetY;
            label         = L"Copilot";
            idleLabel     = L"Installed (not running)";
        }

        // Wrap glow + icon + slash overlay in a 1-cell Grid so they overlap (back to front)
        Grid slot;
        slot.HorizontalAlignment(HorizontalAlignment::Center);
        slot.VerticalAlignment(VerticalAlignment::Center);

        winrt::Windows::UI::Color glowColor;
        if (g_settings.glowColorSet) {
            glowColor = g_settings.glowColorValue;
        } else if (g_settings.activeColorSet) {
            glowColor = g_settings.activeColorValue;
        } else {
            try {
                winrt::Windows::UI::ViewManagement::UISettings ui;
                glowColor = ui.GetColorValue(
                    winrt::Windows::UI::ViewManagement::UIColorType::Accent);
            } catch (...) {
                glowColor = {255, 0, 120, 215};  // Windows blue fallback
            }
        }

        FrameworkElement iconFe = nullptr;
        FrameworkElement glowFe = MakeGlowVisual(glowColor);
        slot.Children().Append(glowFe);

        if (token == L"copilot") {
            bool isDark = (Application::Current().RequestedTheme() == ApplicationTheme::Dark);
            winrt::Windows::UI::Color neutralColor = isDark
                ? winrt::Windows::UI::Color{255, 255, 255, 255}
                : winrt::Windows::UI::Color{255,  30,  30,  30};

            // Try XamlReader to create the real Microsoft Copilot path icon.
            // F1 = EvenOdd fill rule (creates the inner cutout characteristic of the logo).
            // SVG viewBox is 0 0 24 24; Stretch=Uniform scales to the requested icon size.
            static constexpr wchar_t kPathData[] =
                L"F1 M9 23l.073-.001a2.53 2.53 0 01-2.347-1.838l-.697-2.433"
                L"a2.529 2.529 0 00-2.426-1.839h-.497l-.104-.002"
                L"c-4.485 0-2.935-5.278-1.75-9.225l.162-.525"
                L"C2.412 3.99 3.883 1 6.25 1h8.86"
                L"c1.12 0 2.106.745 2.422 1.829l.715 2.453"
                L"a2.53 2.53 0 002.247 1.823l.147.005.534.001"
                L"c3.557.115 3.088 3.745 2.156 7.206l-.113.413"
                L"c-.154.548-.315 1.089-.47 1.607l-.163.525"
                L"C21.588 20.01 20.116 23 17.75 23h-8.75"
                L"zm8.22-15.89l-3.856.001a2.526 2.526 0 00-2.35 1.615"
                L"L9.21 15.04a2.529 2.529 0 01-2.43 1.847"
                L"l3.853.002c1.056 0 1.992-.661 2.361-1.644"
                L"l1.796-6.287a2.529 2.529 0 012.43-1.848z";

            auto toHexColor = [](winrt::Windows::UI::Color c) -> std::wstring {
                wchar_t buf[10];
                swprintf_s(buf, L"#%02X%02X%02X%02X", c.A, c.R, c.G, c.B);
                return std::wstring(buf);
            };

            auto tryMakePath = [&](double sz) -> FrameworkElement {
                try {
                    std::wstring sizeStr = std::to_wstring((int)std::round(sz));
                    std::wstring fillHex = toHexColor(neutralColor);
                    std::wstring xaml =
                        std::wstring(
                            L"<Viewbox"
                            L" xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\""
                            L" Width=\"") + sizeStr +
                        L"\" Height=\"" + sizeStr +
                        L"\">"
                        L"<Path Width=\"24\" Height=\"24\" Stretch=\"Uniform\""
                        L" HorizontalAlignment=\"Center\" VerticalAlignment=\"Center\""
                        L" IsHitTestVisible=\"False\""
                        L" Fill=\"" + fillHex +
                        L"\" Data=\"" + kPathData +
                        L"\"/>"
                        L"</Viewbox>";
                    auto elem = winrt::Windows::UI::Xaml::Markup::XamlReader::Load(xaml);
                    auto vb = elem.try_as<winrt::Windows::UI::Xaml::Controls::Viewbox>();
                    if (!vb) {
                        Wh_Log(L"[Copilot] XamlReader returned a non-Viewbox");
                        return nullptr;
                    }
                    Wh_Log(L"[Copilot] XamlReader OK");
                    return vb.try_as<FrameworkElement>();
                } catch (...) {
                    Wh_Log(L"[Copilot] XamlReader threw");
                    return nullptr;
                }
            };

            // Fallback: 4-pointed sparkle Polygon when XamlReader fails
            auto makeStar = [&](double sz) {
                winrt::Windows::UI::Xaml::Shapes::Polygon p;
                double cx = sz / 2.0, w = sz * 0.14;
                p.Points().Append({(float)cx,       0.0f});
                p.Points().Append({(float)(cx + w), (float)(cx - w)});
                p.Points().Append({(float)sz,        (float)cx});
                p.Points().Append({(float)(cx + w), (float)(cx + w)});
                p.Points().Append({(float)cx,        (float)sz});
                p.Points().Append({(float)(cx - w), (float)(cx + w)});
                p.Points().Append({0.0f,             (float)cx});
                p.Points().Append({(float)(cx - w), (float)(cx - w)});
                p.Width(sz); p.Height(sz);
                p.Stretch(winrt::Windows::UI::Xaml::Media::Stretch::Uniform);
                p.HorizontalAlignment(HorizontalAlignment::Center);
                p.VerticalAlignment(VerticalAlignment::Center);
                p.IsHitTestVisible(false);
                SolidColorBrush br;
                br.Color(neutralColor);
                p.Fill(br);
                return p.try_as<FrameworkElement>();
            };

            auto ip = tryMakePath((double)g_settings.iconSize);
            if (!ip) ip = makeStar((double)g_settings.iconSize);
            iconFe = ip; slot.Children().Append(ip);
        } else {
            auto tb = MakeIconTextBlock(glyph);
            tb.FontFamily(FontFamily(iconFont));
            iconFe = tb;
            slot.Children().Append(tb);
        }

        // Slash overlay — diagonal line across the icon, direction from settings
        double sz = (double)g_settings.iconSize;
        bool falling = (g_settings.slashDirection == L"falling");
        winrt::Windows::UI::Xaml::Shapes::Line slashLine;
        if (falling) {
            slashLine.X1(sz * 0.1);  slashLine.Y1(sz * 0.1);  // top-left
            slashLine.X2(sz * 0.9);  slashLine.Y2(sz * 0.9);  // bottom-right
        } else {
            slashLine.X1(sz * 0.1);  slashLine.Y1(sz * 0.9);  // bottom-left
            slashLine.X2(sz * 0.9);  slashLine.Y2(sz * 0.1);  // top-right
        }
        slashLine.Width(sz);
        slashLine.Height(sz);
        slashLine.Opacity(g_settings.slashOpacity / 100.0);
        slashLine.StrokeThickness(std::max(1.5, sz * 0.09));
        slashLine.StrokeStartLineCap(PenLineCap::Round);
        slashLine.StrokeEndLineCap(PenLineCap::Round);
        slashLine.HorizontalAlignment(HorizontalAlignment::Center);
        slashLine.VerticalAlignment(VerticalAlignment::Center);
        slashLine.IsHitTestVisible(false);
        {
            SolidColorBrush slashBrush;
            if (g_settings.slashColorSet) {
                slashBrush.Color(g_settings.slashColorValue);
            } else {
                bool isDark = (Application::Current().RequestedTheme()
                               == ApplicationTheme::Dark);
                slashBrush.Color(isDark ? winrt::Windows::UI::Color{255, 255, 255, 255}
                                        : winrt::Windows::UI::Color{255,  30,  30,  30});
            }
            slashLine.Stroke(slashBrush);
        }
        slashLine.Visibility(isDisabled ? Visibility::Visible : Visibility::Collapsed);
        slot.Children().Append(slashLine);

        // A transparent background gives the complete slot a stable hit target.
        // This fixes tooltips for the Copilot Viewbox, whose Path intentionally
        // doesn't participate in hit testing, and also makes every icon clickable.
        SolidColorBrush hitTargetBrush;
        hitTargetBrush.Color({0, 0, 0, 0});
        slot.Background(hitTargetBrush);
        SetIconTooltip(slot, label, isActive, blockReason, itemKind, idleLabel);
        auto tappedToken = slot.Tapped(
            [itemKind](auto const&, auto const&) {
                if (!g_unloading)
                    OpenSettingsForItem(itemKind);
            });
        g_slotEventStates.push_back({slot, tappedToken});

        ApplyOffset(slot, offX, offY);

        if (token == L"location")      { g_locSlot = slot; g_locIcon     = iconFe; g_locGlowIcon     = glowFe; g_locSlashIcon     = slashLine; }
        else if (token == L"mic")      { g_micSlot = slot; g_micIcon     = iconFe; g_micGlowIcon     = glowFe; g_micSlashIcon     = slashLine; }
        else if (token == L"camera")   { g_camSlot = slot; g_camIcon     = iconFe; g_camGlowIcon     = glowFe; g_camSlashIcon     = slashLine; }
        else /* copilot */             { g_copilotSlot = slot; g_copilotIcon = iconFe; g_copilotGlowIcon = glowFe; g_copilotSlashIcon = slashLine; }

        grid::Cell placement = grid::GetCell(i, N, layout, cfg);
        Grid::SetRow(slot, placement.row);
        Grid::SetColumn(slot, placement.column);
        if (placement.rowSpan > 1) Grid::SetRowSpan(slot, placement.rowSpan);
        if (placement.columnSpan > 1) Grid::SetColumnSpan(slot, placement.columnSpan);
        if (placement.rowSpan > 1)
            slot.VerticalAlignment(VerticalAlignment::Top);
        if (placement.columnSpan > 1)
            slot.HorizontalAlignment(HorizontalAlignment::Left);
        if (placement.topOffsetUnits || placement.leftOffsetUnits) {
            double pitch = g_settings.iconSize + g_settings.buttonSpacing;
            auto margin = slot.Margin();
            margin.Left += placement.leftOffsetUnits * pitch;
            margin.Top += placement.topOffsetUnits * pitch;
            slot.Margin(margin);
        }

        bar.Children().Append(slot);
    }

    Grid::SetColumn(bar, insertCol);
    gridParent.Children().Append(bar);

    g_syntheticGrid   = bar;
    g_syntheticParent = gridElem;
    g_syntheticColumn = insertCol;

    UpdateSyntheticState();
    Wh_Log(L"[Inject] PrivacyAnchorBar: %d icons, %d cols, %d rows", N, cols, rows);
    return true;
}

static void RemoveSyntheticIcons() {
    // XAML defers removed-subtree teardown to a later UI tick, which can land
    // after the mod DLL unloads. The boxed tooltip and automation-name values on
    // the synthetic icons are implemented in this DLL, so release them before
    // removal (same crash class as folder-menus crash-on-disable).
    for (auto& state : g_slotEventStates) {
        if (!state.element) continue;
        try { state.element.Tapped(state.tappedToken); } catch (...) {}
    }
    g_slotEventStates.clear();

    // Storyboards retain their animation targets. Stop and release them before
    // removing the XAML subtree so no callback can outlive the mod DLL.
    for (auto& state : g_glowAnimationStates) {
        for (auto const& storyboard : state.storyboards) {
            try { storyboard.Stop(); } catch (...) {}
        }
        state.storyboards.clear();
        state.element = nullptr;
        state.running = false;
    }
    g_glowAnimationStates.clear();

    auto clearIconState = [](FrameworkElement const& fe) {
        if (!fe) return;
        try { ToolTipService::SetToolTip(fe, nullptr); } catch (...) {}
        try {
            fe.ClearValue(winrt::Windows::UI::Xaml::Automation::
                          AutomationProperties::NameProperty());
        } catch (...) {}
    };
    clearIconState(g_locSlot ? g_locSlot : g_locIcon);
    clearIconState(g_micSlot ? g_micSlot : g_micIcon);
    clearIconState(g_camSlot ? g_camSlot : g_camIcon);
    clearIconState(g_copilotSlot ? g_copilotSlot : g_copilotIcon);

    auto gridParent = g_syntheticParent ? g_syntheticParent.try_as<Grid>() : nullptr;
    if (!gridParent) {
        g_syntheticGrid    = nullptr;
        g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
        g_locSlot = nullptr; g_micSlot = nullptr; g_camSlot = nullptr; g_copilotSlot = nullptr;
        g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
        g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;
        g_syntheticParent  = nullptr; g_syntheticColumn = -1;
        return;
    }

    int col = g_syntheticColumn;
    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"PrivacyAnchorBar") {
            col = Grid::GetColumn(fe);
            gridParent.Children().RemoveAt(i);
            break;
        }
    }

    if (col >= 0 && (uint32_t)col < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().RemoveAt((uint32_t)col);

    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int c    = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (c > col)                     Grid::SetColumn(fe, c - 1);
        else if (c < col && c + span > col) Grid::SetColumnSpan(fe, span - 1);
    }

    g_syntheticGrid    = nullptr;
    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locSlot = nullptr; g_micSlot = nullptr; g_camSlot = nullptr; g_copilotSlot = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;
    g_syntheticParent  = nullptr; g_syntheticColumn = -1;
    Wh_Log(L"[Remove] PrivacyAnchorBar removed");
}

// ============================================================
// Privacy indicator state tracking
// ============================================================

static void ApplyPrivacyIndicatorBehavior(FrameworkElement iconView) {
    for (auto& s : g_privacyStates)
        if (s.iconViewRef.get() == iconView) return;

    FrameworkElement child = iconView;
    if (!(child = FindChildByName(child, L"ContainerGrid")))    return;
    if (!(child = FindChildByName(child, L"ContentPresenter"))) return;
    if (!(child = FindChildByName(child, L"ContentGrid")))      return;
    child = FindChildByClassName(child, L"SystemTray.TextIconContent");
    if (!child) return;
    if (!(child = FindChildByName(child, L"ContainerGrid")))  return;
    if (!(child = FindChildByName(child, L"Base")))           return;
    if (!(child = FindChildByName(child, L"InnerTextBlock"))) return;

    auto tb = child.try_as<TextBlock>();
    if (!tb) return;
    std::wstring_view text = tb.Text();

    // Log unknown glyphs so new privacy indicator types can be identified during testing.
    if (!text.empty() && text.length() == 1 && !IsPrivacyGlyph(text[0])) {
        Wh_Log(L"[Privacy] Unknown MainStack glyph U+%04X — not a tracked privacy type", (unsigned)text[0]);
        return;
    }
    if (!IsPrivacyText(text)) return;

    PrivacyState::Type type = DetectPrivacyType(text);
    SetPrivacyActive(type, !text.empty());

    PrivacyState state;
    state.iconViewRef  = winrt::make_weak(iconView);
    state.textBlockRef = tb;
    state.type         = type;

    state.textToken = tb.RegisterPropertyChangedCallback(
        TextBlock::TextProperty(),
        [](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            auto tbRef = sender.try_as<TextBlock>();
            if (!tbRef) return;
            std::wstring_view newText = tbRef.Text();
            if (!newText.empty() && newText.length() == 1 && !IsPrivacyGlyph(newText[0])) {
                Wh_Log(L"[Privacy] Unknown glyph change: U+%04X", (unsigned)newText[0]);
                return;
            }
            if (!IsPrivacyText(newText)) return;
            if (newText.empty()) {
                for (auto& s : g_privacyStates) {
                    if (s.textBlockRef.get() == tbRef) {
                        SetPrivacyActive(s.type, false);
                        break;
                    }
                }
            } else {
                auto detectedType = DetectPrivacyType(newText);
                for (auto& s : g_privacyStates) {
                    if (s.textBlockRef.get() == tbRef) {
                        if (s.type != detectedType)
                            SetPrivacyActive(s.type, false);
                        s.type = detectedType;
                        break;
                    }
                }
                SetPrivacyActive(detectedType, true);
            }
        });

    state.visibilityToken = iconView.RegisterPropertyChangedCallback(
        UIElement::VisibilityProperty(),
        [](DependencyObject sender, DependencyProperty) {
            if (g_unloading) return;
            if (!g_settings.suppressNativeIndicators) return;
            auto iconView = sender.try_as<FrameworkElement>();
            if (!iconView || iconView.Visibility() == Visibility::Collapsed) return;
            iconView.Visibility(Visibility::Collapsed);
            iconView.IsHitTestVisible(false);
        });

    g_privacyStates.push_back(std::move(state));
    if (g_settings.suppressNativeIndicators) {
        iconView.Visibility(Visibility::Collapsed);
        iconView.IsHitTestVisible(false);
    } else {
        iconView.IsHitTestVisible(true);
    }
    Wh_Log(L"[Privacy] Tracking indicator type=%d", (int)type);
}

static void ScanMainStack(FrameworkElement mainStack) {
    int count = 0;
    FindChildRecursive(mainStack, [&count](FrameworkElement fe) -> bool {
        if (winrt::get_class_name(fe) != L"SystemTray.IconView") return false;
        const std::wstring name = std::wstring(fe.Name());
        if (name != L"SystemTrayIcon") {
            // Log non-standard names — may correspond to screen capture, presence sensing, etc.
            Wh_Log(L"[Scan] Non-standard IconView name: %s", name.c_str());
            return false;
        }
        ApplyPrivacyIndicatorBehavior(fe);
        count++;
        return false;
    });
    Wh_Log(L"[Scan] MainStack scan complete, tracked %d icon(s)", count);
}

static void ClearPrivacyStates() {
    for (auto& state : g_privacyStates) {
        if (auto tb = state.textBlockRef.get())
            tb.UnregisterPropertyChangedCallback(TextBlock::TextProperty(), state.textToken);
        if (auto iv = state.iconViewRef.get()) {
            if (state.visibilityToken)
                iv.UnregisterPropertyChangedCallback(UIElement::VisibilityProperty(), state.visibilityToken);
            try {
                auto tb = state.textBlockRef.get();
                bool active = tb && !std::wstring_view(tb.Text()).empty();
                iv.IsHitTestVisible(true);
                iv.Visibility(active ? Visibility::Visible : Visibility::Collapsed);
            } catch (...) {}
        }
    }
    g_privacyStates.clear();
    g_locActive.store(false);
    g_micActive.store(false);
    g_camActive.store(false);
    g_locUsage.store(false);
    g_micUsage.store(false);
    g_camUsage.store(false);
    g_locDisabled.store(false);
    g_micDisabled.store(false);
    g_camDisabled.store(false);
    g_locBlockReason.store(PrivacyBlockReason::None);
    g_micBlockReason.store(PrivacyBlockReason::None);
    g_camBlockReason.store(PrivacyBlockReason::None);
    g_copilotInstalled.store(false);
    g_copilotActive.store(false);
    g_copilotDisabled.store(true);
    g_copilotBlockReason.store(PrivacyBlockReason::NotInstalled);
    UpdateSyntheticState();
}

// ============================================================
// Apply
// ============================================================

static void ApplyStyle() {
    Wh_Log(L"[Apply] enter");
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return; }
    g_taskbarWnd = hWnd;

    XamlRoot xamlRoot = nullptr;
    try { xamlRoot = GetTaskbarXamlRoot(hWnd); } catch (...) { return; }
    if (!xamlRoot) { Wh_Log(L"[Apply] XamlRoot unavailable"); return; }

    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return;

    if (!g_syntheticGrid) InjectSyntheticIcons(root);

    auto sysGrid = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (sysGrid) {
        auto mainStack = FindChildByName(sysGrid, L"MainStack");
        if (mainStack) ScanMainStack(mainStack);
    }
}

static void ApplyStyleOnWindowThread() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        g_loadedRevokers.clear();
        ClearPrivacyStates();
        ApplyStyle();
    }, nullptr);
}

static void StopRetryThread() {
    if (g_retryStopEvent) SetEvent(g_retryStopEvent);
    if (g_stateRefreshEvent) SetEvent(g_stateRefreshEvent);
    if (g_retryThread) {
        DWORD result;
        do {
            result = MsgWaitForMultipleObjects(
                1, &g_retryThread, FALSE, INFINITE, QS_SENDMESSAGE);
            if (result == WAIT_OBJECT_0 + 1) {
                MSG message;
                PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
            }
        } while (result == WAIT_OBJECT_0 + 1);
        CloseHandle(g_retryThread);
        g_retryThread = nullptr;
    }
    if (g_retryStopEvent) {
        CloseHandle(g_retryStopEvent);
        g_retryStopEvent = nullptr;
    }
    if (g_stateRefreshEvent) {
        CloseHandle(g_stateRefreshEvent);
        g_stateRefreshEvent = nullptr;
    }
}

// ============================================================
// Hooks
// ============================================================

using IconView_IconView_t = void* (WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* ret = IconView_IconView_Original(pThis);
    if (g_unloading) return ret;

    FrameworkElement iconView = nullptr;
    ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                            winrt::put_abi(iconView));
    if (!iconView) return ret;

    g_loadedRevokers.emplace_back();
    auto it = g_loadedRevokers.end(); --it;
    *it = iconView.Loaded(winrt::auto_revoke_t{},
        [it](winrt::Windows::Foundation::IInspectable const& sender, auto const&) {
            g_loadedRevokers.erase(it);
            if (g_unloading) return;
            auto fe = sender.try_as<FrameworkElement>();
            if (!fe) return;
            if (winrt::get_class_name(fe) == L"SystemTray.IconView" &&
                fe.Name() == L"SystemTrayIcon") {
                if (!g_syntheticGrid) {
                    auto xamlRoot = fe.XamlRoot();
                    if (xamlRoot) {
                        auto root = xamlRoot.Content().try_as<FrameworkElement>();
                        if (root) InjectSyntheticIcons(root);
                    }
                }
                ApplyPrivacyIndicatorBehavior(fe);
            }
        });

    return ret;
}

using LoadLibraryExW_t = HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR path, HANDLE file, DWORD flags) {
    HMODULE h = LoadLibraryExW_Original(path, file, flags);
    if (h && path)
        HandleLoadedModuleIfSystemTray(h, path);
    return h;
}

static bool HookTaskbarDllSymbols() {
    HMODULE h = LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!h) return false;
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        { {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
          &CTaskBand_ITaskListWndSite_vftable },
        { {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
          &CTaskBand_GetTaskbarHost_Original },
        { {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
          &TaskbarHost_FrameHeight_Original },
        { {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
          &std__Ref_count_base__Decref_Original },
    };
    return WindhawkUtils::HookSymbols(h, hooks, ARRAYSIZE(hooks));
}

static bool HookSystemTraySymbols(HMODULE h) {
    WindhawkUtils::SYMBOL_HOOK systemTrayHooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original,
        IconView_IconView_Hook,
        false,
    }};
    return WindhawkUtils::HookSymbols(
        h, systemTrayHooks, ARRAYSIZE(systemTrayHooks));
}

static void HandleLoadedModuleIfSystemTray(HMODULE module,
                                            LPCWSTR fileName) {
    if (!g_systemTrayModuleHooked &&
        GetSystemTrayModuleHandle() == module &&
        !g_systemTrayModuleHooked.exchange(true)) {
        Wh_Log(L"[Hooks] System tray module loaded: %s", fileName);
        if (HookSystemTraySymbols(module)) {
            Wh_ApplyHookOperations();
        } else {
            g_systemTrayModuleHooked = false;
            Wh_Log(L"[Hooks] System tray symbol hooks failed");
        }
    }
}

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] Privacy Anchor v0.9");
    LoadSettings();

    if (!HookTaskbarDllSymbols()) {
        Wh_Log(L"[Init] taskbar.dll symbols failed");
        return FALSE;
    }

    if (HMODULE module = GetSystemTrayModuleHandle()) {
        if (!HookSystemTraySymbols(module)) {
            Wh_Log(L"[Init] System tray symbol hooks failed");
            return FALSE;
        }
        g_systemTrayModuleHooked = true;
    } else {
        HMODULE kernelbase = GetModuleHandleW(L"kernelbase.dll");
        auto loadLibraryExW = kernelbase
            ? reinterpret_cast<LoadLibraryExW_t>(
                  GetProcAddress(kernelbase, "LoadLibraryExW"))
            : nullptr;
        if (!loadLibraryExW ||
            !WindhawkUtils::SetFunctionHook(
                loadLibraryExW,
                LoadLibraryExW_Hook,
                &LoadLibraryExW_Original)) {
            Wh_Log(L"[Init] LoadLibraryExW hook unavailable");
            return FALSE;
        }
    }

    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_systemTrayModuleHooked) {
        if (HMODULE module = GetSystemTrayModuleHandle()) {
            if (!g_systemTrayModuleHooked.exchange(true)) {
                if (HookSystemTraySymbols(module))
                    Wh_ApplyHookOperations();
                else
                    g_systemTrayModuleHooked = false;
            }
        }
    }
    if (g_systemTrayModuleHooked)
        ApplyStyleOnWindowThread();

    g_retryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_stateRefreshEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_retryStopEvent || !g_stateRefreshEvent) {
        if (g_retryStopEvent) { CloseHandle(g_retryStopEvent); g_retryStopEvent = nullptr; }
        if (g_stateRefreshEvent) { CloseHandle(g_stateRefreshEvent); g_stateRefreshEvent = nullptr; }
        return;
    }
    g_retryThread = CreateThread(nullptr, 0, [](void* param) -> DWORD {
        UNREFERENCED_PARAMETER(param);
        HANDLE stop = g_retryStopEvent;
        HANDLE refresh = g_stateRefreshEvent;
        // Phase 1: retry injection up to 5×
        for (int i = 0; i < 5 && !g_unloading; i++) {
            if (WaitForSingleObject(stop, 2000) != WAIT_TIMEOUT) return 0;
            if (g_syntheticGrid) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyStyleOnWindowThread();
        }
        // Phase 2: event-driven privacy state. Registry, access, device,
        // microphone, and camera notifications wake this thread with a domain
        // bitmask. Timers are only used for Copilot process activity, a
        // five-minute health reconciliation, and backed-off setup retries.
        if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) return 0;
        MicPrivacyMonitor micMonitor;
        CameraPrivacyMonitor cameraMonitor;
        DeviceStateMonitor deviceMonitor;
        RegistryChangeMonitor registryMonitor;

        registryMonitor.AddWatch(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\CapabilityAccessManager\\ConsentStore\\location",
            RefreshLocationState | RefreshLocationUsage,
            L"HKCU location ConsentStore");
        registryMonitor.AddWatch(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\CapabilityAccessManager\\ConsentStore\\location",
            RefreshLocationState | RefreshLocationUsage,
            L"HKLM location ConsentStore");
        registryMonitor.AddWatch(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\CapabilityAccessManager\\ConsentStore\\microphone",
            RefreshMicrophoneState | RefreshMicrophoneUsage,
            L"HKCU microphone ConsentStore");
        registryMonitor.AddWatch(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\CapabilityAccessManager\\ConsentStore\\microphone",
            RefreshMicrophoneState | RefreshMicrophoneUsage,
            L"HKLM microphone ConsentStore");
        registryMonitor.AddWatch(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\CapabilityAccessManager\\ConsentStore\\webcam",
            RefreshCameraState | RefreshCameraUsage,
            L"HKCU webcam ConsentStore");
        registryMonitor.AddWatch(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows\\CurrentVersion"
            L"\\CapabilityAccessManager\\ConsentStore\\webcam",
            RefreshCameraState | RefreshCameraUsage,
            L"HKLM webcam ConsentStore");
        registryMonitor.AddWatch(
            HKEY_CURRENT_USER, L"Software\\Policies\\Microsoft",
            RefreshLocationState | RefreshCopilotState,
            L"HKCU Microsoft policies");
        registryMonitor.AddWatch(
            HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft",
            RefreshLocationState | RefreshCopilotState,
            L"HKLM Microsoft policies");
        registryMonitor.AddWatch(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\lfsvc",
            RefreshLocationState, L"location service configuration");
        registryMonitor.AddWatch(
            HKEY_CURRENT_USER,
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",
            RefreshCopilotState, L"Explorer taskbar settings");
        registryMonitor.AddWatch(
            HKEY_CURRENT_USER,
            L"Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows"
            L"\\CurrentVersion\\AppModel\\Repository",
            RefreshCopilotState, L"HKCU AppModel repository");
        registryMonitor.AddWatch(
            HKEY_LOCAL_MACHINE,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModel",
            RefreshCopilotState, L"HKLM AppModel repository");

        micMonitor.Init();
        cameraMonitor.Init();
        deviceMonitor.Init();
        DWORD initialRegistrationFlags = registryMonitor.RefreshRegistrations();
        DWORD initialFlags = g_pendingRefreshFlags.exchange(RefreshNone) |
                             RefreshAll | initialRegistrationFlags;
        Wh_Log(L"[Refresh] Phase 2 starting — initial state reconciliation");
        UpdatePrivacyStates(initialFlags);
        Wh_Log(L"[Refresh] Baseline: loc=%d mic=%d cam=%d copInst=%d copAct=%d copDis=%d",
               g_locDisabled.load(), g_micDisabled.load(), g_camDisabled.load(),
               g_copilotInstalled.load(), g_copilotActive.load(), g_copilotDisabled.load());

        static constexpr ULONGLONG kCopilotIntervalMs = 60 * 1000;
        static constexpr ULONGLONG kHealthIntervalMs = 5 * 60 * 1000;
        ULONGLONG nextCopilotCheck = GetTickCount64() + kCopilotIntervalMs;
        ULONGLONG nextHealthCheck = GetTickCount64() + kHealthIntervalMs;

        auto delayUntil = [](ULONGLONG deadline) -> DWORD {
            ULONGLONG now = GetTickCount64();
            if (now >= deadline)
                return 0;
            return static_cast<DWORD>(std::min<ULONGLONG>(
                deadline - now, static_cast<ULONGLONG>(INFINITE - 1)));
        };

        while (!g_unloading) {
            std::vector<HANDLE> waitEvents{stop, refresh};
            registryMonitor.AppendWaitHandles(waitEvents);

            DWORD timeout = delayUntil(nextHealthCheck);
            if (g_copilotItemEnabled.load())
                timeout = std::min(timeout, delayUntil(nextCopilotCheck));
            timeout = std::min(timeout, cameraMonitor.NextActionDelayMs());
            timeout = std::min(timeout, registryMonitor.NextActionDelayMs());

            DWORD wait = WaitForMultipleObjects(
                static_cast<DWORD>(waitEvents.size()), waitEvents.data(),
                FALSE, timeout);
            if (wait == WAIT_OBJECT_0) break;
            if (wait == WAIT_FAILED) {
                Wh_Log(L"[Refresh] WaitForMultipleObjects failed error=%u",
                       GetLastError());
                break;
            }

            DWORD flags = RefreshNone;
            if (wait == WAIT_OBJECT_0 + 1) {
                flags |= g_pendingRefreshFlags.exchange(RefreshNone);
            } else if (wait >= WAIT_OBJECT_0 + 2 &&
                       wait < WAIT_OBJECT_0 + waitEvents.size()) {
                HANDLE signaled = waitEvents[wait - WAIT_OBJECT_0];
                flags |= registryMonitor.HandleSignaled(signaled);
            }

            if (flags & RefreshMonitorSetup) {
                registryMonitor.ResetFailedRetries();
                cameraMonitor.ResetFailedRetry();
                flags &= ~RefreshMonitorSetup;
            }
            cameraMonitor.Refresh();
            flags |= g_pendingRefreshFlags.exchange(RefreshNone);

            if (flags & RefreshMonitorSetup) {
                registryMonitor.ResetFailedRetries();
                cameraMonitor.ResetFailedRetry();
                flags &= ~RefreshMonitorSetup;
            }
            flags |= registryMonitor.RefreshRegistrations();

            ULONGLONG now = GetTickCount64();
            if (g_copilotItemEnabled.load() && now >= nextCopilotCheck) {
                flags |= RefreshCopilotActivity;
                nextCopilotCheck = now + kCopilotIntervalMs;
            } else if (!g_copilotItemEnabled.load()) {
                nextCopilotCheck = now + kCopilotIntervalMs;
            }
            if (now >= nextHealthCheck) {
                flags |= RefreshAll;
                nextHealthCheck = now + kHealthIntervalMs;
                Wh_Log(L"[Refresh] Five-minute health reconciliation");
            }

            UpdatePrivacyStates(flags & RefreshAll);
        }
        deviceMonitor.Cleanup();
        cameraMonitor.Cleanup();
        micMonitor.Cleanup();
        registryMonitor.Cleanup();
        CoUninitialize();
        return 0;
    }, nullptr, 0, nullptr);
    if (!g_retryThread) {
        CloseHandle(g_retryStopEvent); g_retryStopEvent = nullptr;
        CloseHandle(g_stateRefreshEvent); g_stateRefreshEvent = nullptr;
    }
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");
    StopRetryThread();
    // Loaded revokers wrap WinRT objects that must be destroyed on the UI
    // thread — clear them inside RunFromWindowThread, not here.
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(hWnd, [](void*) {
            g_loadedRevokers.clear();
            ClearPrivacyStates();
            RemoveSyntheticIcons();
        }, nullptr);
    } else {
        g_loadedRevokers.clear();
        ClearPrivacyStates();
        RemoveSyntheticIcons();
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] order=%s rows=%d cols=%d suppressNative=%d cameraHardware=%d cameraItem=%d copilotItem=%d glow=%d/%s opacity=%d reach=%d speed=%d",
           g_settings.itemOrder.c_str(), g_settings.gridRows,
           g_settings.gridColumns, g_settings.suppressNativeIndicators ? 1 : 0,
           g_cameraHardwareDetectionEnabled.load() ? 1 : 0,
           g_cameraItemEnabled.load() ? 1 : 0,
           g_copilotItemEnabled.load() ? 1 : 0,
           g_settings.glowEnabled ? 1 : 0, g_settings.glowStyle.c_str(),
           g_settings.glowOpacity, g_settings.glowSize,
           g_settings.glowSpeed);

    RequestStateRefresh(RefreshAll | RefreshMonitorSetup);

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) {
        ClearPrivacyStates();
        RemoveSyntheticIcons();
        ApplyStyle();
    }, nullptr);
}
