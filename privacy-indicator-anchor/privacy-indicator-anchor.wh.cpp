// ==WindhawkMod==
// @id              tray-privacy-indicator-anchor
// @name            Tray Privacy Indicator Anchor
// @description     Permanently shows location/microphone/camera/Copilot icons in the system tray — dim when idle, bright when in use — preventing taskbar layout shifts.
// @version         2.0
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

## Gallery

Idle location and microphone placeholders reserve their tray space without
demanding attention:

![Idle location and microphone placeholders](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/location-mic-availble-not-in-use.png)

All four unavailable indicators in a single row:

![All four privacy indicators unavailable in one row](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/all-4-disabled.png)

The same four indicators in a compact Smart Grid layout:

![All four unavailable indicators in a compact grid](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/all-4-disabled-grid.png)

Activity highlighted in red — the microphone in use, and the slashed camera
reporting attempted use while its hardware switch blocks it:

![Active microphone in red beside a red slashed camera blocked by its hardware switch](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/camera-mic-in-use-highlighted.png)

The active glow treatment provides a more emphatic alternative:

![Active microphone and camera with glow](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/cam-mic-in-use-highlight-glow.png)

Active or requested pathways can remain conspicuous even while blocked:

![Requested or active privacy pathways shown while blocked](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/requested-or-active-pathways-disabled.png)

The location tooltip explains the access-denied reason and opens the matching
Windows privacy page when clicked:

![Location evidence tooltip and Windows Location settings](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/location-tooltip-and-win-settings.png)

Microphone evidence distinguishes an endpoint mute from privacy denial:

![Microphone endpoint-mute evidence tooltip](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/mic-tooltip.png)

Supported camera drivers report their hardware privacy-control evidence:

![Camera hardware privacy-control evidence tooltip](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/camera-tooltip.png)

Copilot reports its installation state and links to the relevant settings:

![Copilot installation-state tooltip](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/privacy-indicator-anchor/assets/copilot-tooltip.png)

## Features

- Persistent placeholder icons for location, microphone, camera, and Copilot
- Idle opacity setting so inactive icons can be subtle but still reserve space
- Configurable icon order with `location`, `mic`, `camera`, and `copilot` tokens
- Six Smart Grid modes with balanced, vertical-pack, and horizontal-pack layouts
- Short row/column placement and alignment controls
- Tray placement before icons, before OmniButton, before clock, after clock, or after Show Desktop
- Experimental placement immediately left or right of Start
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

`gridMode` selects Smart automatic, Single row, Single column, Fixed rows,
Fixed columns, or Fixed rows and columns. In Smart automatic mode,
`smartLayout` chooses Balanced, Pack vertical, or Pack horizontal. The default
Balanced layout picks the least-skewed shape that fits the live taskbar height.

`gridRows` and `gridColumns` provide the requested dimensions for the fixed
modes; `0` leaves that axis automatic. Smart automatic uses the live taskbar
height and icon pitch as its row capacity rather than treating these values as
an implicit mode selector.

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

## Placement

The five tray positions reserve a dedicated system-tray column. The
experimental `leftOfStart` and `rightOfStart` positions instead place the
owned indicator group beside Start and reserve matching room in the centered
taskbar items area. These Start-adjacent modes may need adjustment on future
Windows builds or with other mods that also reposition Start.

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

## Notes

Camera hardware-switch detection and the Copilot indicator are experimental
because Windows exposes those states differently across devices and builds.

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
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- Placement:
  - Position: "beforeOmni"
    $name: Position
    $description: Where to place the group in the Windows 11 taskbar.
    $options:
    - "beforeIcons": "Before notification icons"
    - "beforeOmni": "Before network, volume, and battery"
    - "beforeClock": "Before clock"
    - "afterClock": "After clock"
    - "afterShowDesktop": "After Show Desktop"
    - "leftOfStart": "Left of Start (experimental)"
    - "rightOfStart": "Right of Start (experimental)"
  $name: Placement

- Content:
  - Location: true
    $name: Location icon
  - Microphone: true
    $name: Microphone icon
  - Camera: true
    $name: Camera icon
    $description: Experimental. Availability depends on what the camera driver reports.
  - Copilot: true
    $name: Copilot icon
    $description: Experimental. Reflects Copilot installation, policy, and process activity.
  $name: Content

- Layout:
  - Arrangement: "auto"
    $name: Arrangement
    $description: >-
      "auto" fits the enabled icons to the available taskbar height. Anything
      else is an explicit layout: names side by side with "|", stacked with
      ",", and grouped with parentheses - "location, mic | camera, copilot"
      is a 2x2 block. Tokens are location, mic (or microphone), camera, and
      copilot. Append a pixel offset to nudge one icon, "mic[+2,-1]", or a
      whole group, "(location, mic)[3,0]". Every time "auto" is applied, its
      generated arrangement is written to the Windhawk log so you can paste it
      here and edit it. A parse error is logged and falls back to automatic.
  - FillOrder: "rows"
    $name: Fill order
    $description: Used by "auto". Whether icons fill across rows or down columns first.
    $options:
    - "rows": "Fill rows first (left to right, then down)"
    - "columns": "Fill columns first (top to bottom, then right)"
  - Justify: "center"
    $name: Short row or column
    $description: Used by "auto". How a ragged last row or column is aligned.
    $options:
    - "start": "Start (top for columns, left for rows)"
    - "center": "Center"
    - "end": "End (bottom for columns, right for rows)"
  - NewItems: "append"
    $name: Newly enabled icons
    $description: >-
      What happens when you enable an icon that your own arrangement does not
      name. Only applies to a written arrangement - "auto" always includes
      every enabled icon.
    $options:
    - "append": "Add them after the arrangement"
    - "ignore": "Leave them out until I add them"
  $name: Layout

- Size:
  - ItemSize: 16
    $name: Icon size (px)
  - ItemSpacing: 4
    $name: Icon spacing (px)
    $description: Gap between icons along each axis.
  $name: Size

- Adjust:
  - PadX: 0
    $name: Horizontal padding (px)
    $description: Space reserved on both sides of the icon group.
  - PadY: 0
    $name: Vertical padding (px)
    $description: Space reserved above and below the icon group.
  - OffsetX: 0
    $name: Horizontal offset (px)
    $description: Moves the whole group. Does not reserve space.
  - OffsetY: 0
    $name: Vertical offset (px)
    $description: Moves the whole group up (negative) or down (positive).
  $name: Adjust

- Surface:
  - IdleOpacity: 50
    $name: Idle opacity (%)
    $description: 0 is invisible with space reserved; 100 is full brightness.
  - ActiveOpacity: 100
    $name: Active opacity (%)
  - GlowEnabled: false
    $name: Glow when active
    $description: Adds emphasis behind an active icon without changing its reserved size.
  - GlowOpacity: 40
    $name: Glow opacity (%)
  - SlashColor: ""
    $name: Slash color
    $description: >-
      Hex (#RRGGBB or #AARRGGBB), accent / accentLight / accentDark, or
      transparent. Empty uses the system foreground color.
  - SlashDirection: "falling"
    $name: Slash direction
    $options:
    - "falling": 'Falling (\ upper-left to lower-right)'
    - "rising": "Rising (/ lower-left to upper-right)"
  - SlashOpacity: 100
    $name: Slash opacity (%)
  - IdleColor: ""
    $name: Idle icon color
    $description: Hex, accent / accentLight / accentDark, or transparent. Empty uses the system foreground.
  - ActiveColor: ""
    $name: Active icon color
    $description: Hex, accent / accentLight / accentDark, or transparent. Empty uses the system foreground.
  - DisabledOpacity: 50
    $name: Disabled icon opacity (%)
  - DisabledColor: ""
    $name: Disabled icon color
    $description: Hex, accent / accentLight / accentDark, or transparent. Empty uses the system foreground.
  - AlertWhenBlockedAndActive: true
    $name: Emphasize blocked activity
    $description: Keep the active color and glow beneath the slash when Windows also reports activity.
  - GlowStyle: "radiate"
    $name: Glow style
    $options:
    - "steady": "Steady halo"
    - "pulse": "Breathing halo"
    - "radiate": "Radiating rings"
  - GlowColor: ""
    $name: Glow color
    $description: Empty follows Active icon color, then the Windows accent color.
  - GlowSize: 220
    $name: Glow reach (%)
  - GlowSpeed: 1200
    $name: Glow cycle (ms)
  $name: Surface

- Behavior:
  - CameraHardwareDetection: false
    $name: Monitor camera hardware privacy control
    $description: >-
      Experimental and opt-in. Uses the Windows 11 CameraOcclusionInfo driver
      signal. This initializes the default camera controller in SharedReadOnly
      mode but never starts preview or frame capture. Turn it off if a camera
      activates its LED or behaves poorly while monitored.
  - SuppressNativeIndicators: true
    $name: Suppress Windows privacy indicators
    $description: >-
      Hides Windows' pop-in indicators and mirrors their state into these
      stable placeholders. Turn off temporarily when testing native behavior.
  $name: Behavior
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
#include <exception>
#include <functional>
#include <list>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <windhawk_utils.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;

// ============================================================
// Nested group layout
// Template block: _templates/nested-group-layout.h v2.4 (verbatim copy —
// keep in sync with the template; Windhawk mods are single-file).
// ============================================================

// Copy-source template v2.0: nested group layout — pixel-space placement of
// named items described by ONE layout expression. No Windows or WinRT
// dependency; the caller supplies pixel sizes and the taskbar metrics.
//
// This is the only element arranger in the mod family. It backs a single
// user-facing setting, `Layout.Arrangement`, whose default value is the word
// `auto`:
//
//   auto            -> ChooseShape() picks rows x columns from the available
//                      taskbar height, BuildGridExpression() emits the
//                      equivalent expression, and the mod logs it.
//   anything else   -> that string IS the layout.
//
// Because the Windhawk settings API is read-only (windhawk_api.h has no
// setter), a mod can never fill the field in for the user. Logging the
// expansion is the supported path: the user pastes the logged expression back
// into the same field to take manual control. There is one field and one
// string, so nothing can drift out of sync with anything else.
//
// Grammar:
//   expr  := stack ('|' stack)*      '|' places groups side by side
//   stack := unit (',' unit)*        ',' stacks units top to bottom
//   unit  := leaf | '(' expr ')'     parens nest, orientation never flips
//   leaf  := token ('[' dx ',' dy ']')?
//
// "1, 2 | 3, 4" is a 2x2 block. "a | b, c | d" is three columns with b over c
// (the diamond). Nesting is arbitrary: "a | (b, (c | d)), e | f". '|' always
// means horizontal and ',' always means vertical, at every depth — there is no
// primary-axis setting to reason about.
//
// OFFSETS ride in the expression: "1[+2,-1] | 2 | 3" shifts item 1 two pixels
// right and one up. A parenthesized group takes one too — "(1, 2)[3,0] | 3"
// moves that whole column. Offsets are cosmetic: they move their own leaf or
// their own group's contents, and change neither the measured size nor any
// neighbor's position. This replaces every keyed per-item offset setting;
// there is no second string to maintain.
//
// A separator is always required: "1 (2 | 3)" is a parse error, not an
// implicit "1 | (2 | 3)". Silently reinterpreting a missing separator would
// turn a typo into a different layout instead of a logged, recoverable error.
//
// Tokens are caller-defined names resolved to pixel sizes by a callback. A
// token that resolves to an empty size (width or height <= 0) is skipped and
// consumes no space, so absent items collapse out of the arrangement.
//
// PADDING vs OFFSET. Config.padX / padY are symmetric outer padding: they
// participate in layout and are included in the returned totalSize. Group
// offset (Adjust.OffsetX / OffsetY) is a visual translation that must NOT
// reserve space, so it is deliberately not handled here — the mod applies it
// to the container it places, after this arranger has sized the group.
//
// Every group is centered on its cross axis by Config.justify.

#include <algorithm>
#include <cwctype>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>

namespace windhawk_mod_templates::nested_group_layout {

enum class Axis { Horizontal, Vertical };  // node orientation, not a setting
enum class Justify { Start, Center, End };
enum class FillOrder { Rows, Columns };

// An item is sized either absolutely (width x height) or RELATIVE TO THE AXIS
// its group happens to lay out along. Axis-relative sizing exists because an
// item like a Task View button should be "as wide as it needs and as tall as
// the buttons beside it" when it is a column, and the mirror image when it is
// a row — and in a hand-written arrangement the mod cannot know which it will
// be. The parent group knows its own axis, so it resolves this at measure and
// arrange time:
//
//   thickness — extent ALONG the group's axis (its width as a column, its
//               height as a row)
//   cross     — extent ACROSS the group's axis; 0 means fill, i.e. match
//               whatever the rest of the group measures
struct Size {
    double width = 0.0;
    double height = 0.0;
    bool axisRelative = false;
    double thickness = 0.0;
    double cross = 0.0;

    bool Empty() const {
        return axisRelative ? thickness <= 0.0
                            : (width <= 0.0 || height <= 0.0);
    }
};

// Size an item against its group's axis. cross = 0 fills the group.
inline Size AlongAxis(double thickness, double cross = 0.0) {
    Size size;
    size.axisRelative = true;
    size.thickness = thickness;
    size.cross = cross;
    return size;
}

// Cosmetic per-leaf nudge parsed from the expression's "[dx,dy]" suffix.
struct Offset {
    double x = 0.0;
    double y = 0.0;
};

struct Config {
    double spacing = 0.0;
    Justify justify = Justify::Center;
    double padX = 0.0;  // reserved on BOTH left and right
    double padY = 0.0;  // reserved on BOTH top and bottom
};

struct Placement {
    std::wstring token;
    double x = 0.0;
    double y = 0.0;
    Size size;
};

struct Node {
    std::wstring token;            // non-empty = leaf
    Offset offset;                 // from the "[dx,dy]" suffix; leaf or group
    std::vector<Node> children;    // group children, laid along axis
    Axis axis = Axis::Horizontal;  // group axis (unused for leaves)
};

// Where an arrangement stopped making sense, and what was expected there.
// Report both: a hand-edited expression is much easier to fix with a column
// number than with "did not parse".
struct ParseError {
    size_t position = 0;
    std::wstring expected;
};

class Parser {
public:
    explicit Parser(std::wstring const& text) : text_(text) {}

    bool Run(Node& root) {
        position_ = 0;
        valid_ = true;
        root = ParseExpr();
        SkipSpace();
        if (valid_ && position_ < text_.size())
            Fail(position_, L"a separator ('|' or ',') or end of arrangement");
        return valid_;
    }

    ParseError const& Error() const { return error_; }

private:
    void Fail(size_t position, wchar_t const* expected) {
        if (valid_) {  // keep the first failure; later ones are fallout
            valid_ = false;
            error_ = {position, expected};
        }
    }

    Node ParseExpr() {
        Node node;
        node.axis = Axis::Horizontal;
        node.children.push_back(ParseStack());
        while (Peek() == L'|') {
            ++position_;
            node.children.push_back(ParseStack());
        }
        return node;
    }

    Node ParseStack() {
        Node node;
        node.axis = Axis::Vertical;
        node.children.push_back(ParseUnit());
        while (Peek() == L',') {
            ++position_;
            node.children.push_back(ParseUnit());
        }
        return node;
    }

    Node ParseUnit() {
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L'(') {
            ++position_;
            Node inner = ParseExpr();
            SkipSpace();
            if (position_ < text_.size() && text_[position_] == L')')
                ++position_;
            else
                Fail(position_, L"a closing ')'");
            // A group takes an offset too, moving everything inside it.
            if (position_ < text_.size() && text_[position_] == L'[')
                inner.offset = ParseOffset();
            return inner;
        }

        Node leaf;
        size_t start = position_;
        while (position_ < text_.size() && !IsDelimiter(text_[position_]))
            ++position_;
        leaf.token = text_.substr(start, position_ - start);
        if (leaf.token.empty()) {
            Fail(position_, L"a name");
            return leaf;
        }
        if (position_ < text_.size() && text_[position_] == L'[')
            leaf.offset = ParseOffset();
        return leaf;
    }

    // "[dx,dy]" — signs optional, spaces allowed, both components required.
    Offset ParseOffset() {
        ++position_;  // consume '['
        Offset offset;
        offset.x = ParseNumber();
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L',')
            ++position_;
        else
            Fail(position_, L"a ',' between the x and y offsets");
        offset.y = ParseNumber();
        SkipSpace();
        if (position_ < text_.size() && text_[position_] == L']')
            ++position_;
        else
            Fail(position_, L"a closing ']'");
        return offset;
    }

    double ParseNumber() {
        SkipSpace();
        wchar_t* end = nullptr;
        double value = std::wcstod(text_.c_str() + position_, &end);
        size_t consumed = end ? (size_t)(end - (text_.c_str() + position_)) : 0;
        if (!consumed) {
            Fail(position_, L"a number");
            return 0.0;
        }
        position_ += consumed;
        return value;
    }

    static bool IsDelimiter(wchar_t c) {
        return c == L'|' || c == L',' || c == L'(' || c == L')' ||
               c == L'[' || c == L']' || iswspace(c);
    }

    wchar_t Peek() {
        SkipSpace();
        return position_ < text_.size() ? text_[position_] : L'\0';
    }

    void SkipSpace() {
        while (position_ < text_.size() && iswspace(text_[position_]))
            ++position_;
    }

    std::wstring const& text_;
    size_t position_ = 0;
    bool valid_ = true;
    ParseError error_;
};

inline bool Parse(std::wstring const& text, Node& root,
                  ParseError* error = nullptr) {
    Parser parser(text);
    bool ok = parser.Run(root);
    if (!ok && error)
        *error = parser.Error();
    return ok;
}

// ---- Token vocabulary -------------------------------------------------------
//
// A token is an item's stable IDENTITY, never its displayed label. Labels are
// not unique, can contain the expression's own delimiters, can be empty or an
// emoji, and renaming one would silently break an arrangement the user wrote.
// Each mod declares its vocabulary and documents it:
//
//   fixed set     -> semantic names: wifi, volume, battery, percent, clock
//   dynamic set   -> 1, 2, 3, ... because the set changes at runtime
//   either        -> an extra named item such as "master"
//
// A dynamic mod may accept a readable alias for a number (desktop2 == 2). Log
// the token-to-label map next to the arrangement so a user can tell which
// number is which item without the arrangement depending on the labels.
//
// Matching is case-insensitive: someone typing "Wifi" means wifi.

inline bool TokenIs(std::wstring const& token, wchar_t const* name) {
    size_t i = 0;
    for (; i < token.size() && name[i]; ++i)
        if (towlower(token[i]) != towlower(name[i]))
            return false;
    return i == token.size() && !name[i];
}

// "desktop2" -> 2 with prefix L"desktop"; 0 when the token does not match.
inline int TokenIndexWithPrefix(std::wstring const& token,
                                wchar_t const* prefix) {
    size_t i = 0;
    for (; prefix[i]; ++i)
        if (i >= token.size() || towlower(token[i]) != towlower(prefix[i]))
            return 0;
    if (i >= token.size())
        return 0;
    int value = 0;
    for (; i < token.size(); ++i) {
        if (token[i] < L'0' || token[i] > L'9')
            return 0;
        value = value * 10 + (token[i] - L'0');
    }
    return value;
}

using SizeResolver = std::function<Size(std::wstring const&)>;

inline Size Measure(Node const& node, Config const& config,
                    SizeResolver const& resolve) {
    if (!node.token.empty())
        return resolve(node.token);

    // The grammar wraps every unit in a group, so most groups have a single
    // child. Such a group IS its child — pass the size through verbatim, or an
    // axis-relative child would be flattened into a concrete size by its own
    // wrapper before the real parent ever sees it.
    {
        Node const* only = nullptr;
        int visible = 0;
        for (auto const& child : node.children) {
            if (Measure(child, config, resolve).Empty())
                continue;
            only = &child;
            if (++visible > 1)
                break;
        }
        if (visible == 1)
            return Measure(*only, config, resolve);
    }

    double main = 0.0;
    double cross = 0.0;
    double fillFallback = 0.0;
    int placed = 0;
    for (auto const& child : node.children) {
        Size size = Measure(child, config, resolve);
        if (size.Empty())
            continue;
        double childMain, childCross;
        if (size.axisRelative) {
            childMain = size.thickness;
            // A filling item takes its cross extent FROM the group, so it must
            // not drive the group's cross size — otherwise it would size itself.
            childCross = size.cross;
            fillFallback = std::max(fillFallback, size.thickness);
        } else {
            childMain =
                node.axis == Axis::Horizontal ? size.width : size.height;
            childCross =
                node.axis == Axis::Horizontal ? size.height : size.width;
        }
        main += (placed ? config.spacing : 0.0) + childMain;
        cross = std::max(cross, childCross);
        ++placed;
    }
    if (!placed)
        return {};
    // Degenerate case: every child fills, so nothing established a cross size.
    // Fall back to the largest thickness rather than collapsing the group.
    if (cross <= 0.0)
        cross = fillFallback;
    return node.axis == Axis::Horizontal ? Size{main, cross}
                                         : Size{cross, main};
}

// Resolve a child's size against its parent group's axis, so an axis-relative
// item becomes concrete width x height.
inline Size ConcreteSize(Size const& size, Axis axis, Size const& groupTotal) {
    if (!size.axisRelative)
        return size;
    double groupCross =
        axis == Axis::Horizontal ? groupTotal.height : groupTotal.width;
    double cross = size.cross > 0.0 ? size.cross : groupCross;
    return axis == Axis::Horizontal ? Size{size.thickness, cross}
                                    : Size{cross, size.thickness};
}

inline void Arrange(Node const& node, Config const& config,
                    SizeResolver const& resolve, double x, double y,
                    std::vector<Placement>& out,
                    Size const* resolvedSize = nullptr) {
    if (!node.token.empty()) {
        Size size = resolvedSize ? *resolvedSize : resolve(node.token);
        if (!size.Empty())
            out.push_back(
                {node.token, x + node.offset.x, y + node.offset.y, size});
        return;
    }

    Size total = Measure(node, config, resolve);
    if (total.Empty())
        return;
    // A group's own offset moves everything inside it and nothing outside.
    x += node.offset.x;
    y += node.offset.y;

    // Single-child group: forward the size the real parent already resolved,
    // so axis-relative sizing survives the grammar's per-unit wrapper.
    {
        Node const* only = nullptr;
        int visible = 0;
        for (auto const& child : node.children) {
            if (Measure(child, config, resolve).Empty())
                continue;
            only = &child;
            if (++visible > 1)
                break;
        }
        if (visible == 1) {
            Arrange(*only, config, resolve, x, y, out, resolvedSize);
            return;
        }
    }

    double cursor = node.axis == Axis::Horizontal ? x : y;
    for (auto const& child : node.children) {
        Size measured = Measure(child, config, resolve);
        if (measured.Empty())
            continue;
        Size size = ConcreteSize(measured, node.axis, total);
        double unused = node.axis == Axis::Horizontal
                            ? total.height - size.height
                            : total.width - size.width;
        double crossOffset = config.justify == Justify::Center ? unused / 2.0
                             : config.justify == Justify::End  ? unused
                                                               : 0.0;
        if (node.axis == Axis::Horizontal) {
            Arrange(child, config, resolve, cursor, y + crossOffset, out,
                    &size);
            cursor += size.width + config.spacing;
        } else {
            Arrange(child, config, resolve, x + crossOffset, cursor, out,
                    &size);
            cursor += size.height + config.spacing;
        }
    }
}

// Parse + measure + arrange in one call. Returns false only on a parse error
// (unbalanced parentheses, malformed offset, trailing garbage) — the caller
// should then fall back to the auto expression and log that it did.
// placements come back in expression order; totalSize is the group's bounding
// box INCLUDING outer padding. A per-item offset shifts its leaf without
// changing totalSize or any neighbor.
inline bool Compute(std::wstring const& text, Config const& config,
                    SizeResolver const& resolve,
                    std::vector<Placement>& placements, Size& totalSize,
                    ParseError* error = nullptr) {
    Node root;
    if (!Parse(text, root, error))
        return false;
    Size inner = Measure(root, config, resolve);
    placements.clear();
    if (inner.Empty()) {
        // No visible items: an empty group has no padded box either.
        totalSize = {};
        return true;
    }
    if (inner.axisRelative) {
        // The whole arrangement is one axis-relative item, so there is no group
        // for it to fill against; square it off on its own thickness.
        double cross = inner.cross > 0.0 ? inner.cross : inner.thickness;
        inner = Size{inner.thickness, cross};
    }
    Arrange(root, config, resolve, config.padX, config.padY, placements,
            &inner);
    totalSize = {inner.width + config.padX * 2.0,
                 inner.height + config.padY * 2.0};
    return true;
}

// ---- Taskbar metrics --------------------------------------------------------
//
// The taskbar rect comes from GetWindowRect in PHYSICAL pixels while every XAML
// size is a DIP. Dividing one by the other is the DPI bug flagged on PR #4855
// (blocking) and #4843. The mod supplies the raw numbers:
//
//   RECT r{}; GetWindowRect(hTaskbarWnd, &r);
//   int rows = AvailableRows(r.bottom - r.top, GetDpiForWindow(hTaskbarWnd),
//                            itemHeight, spacing);

inline double PixelsToDip(double physicalPixels, unsigned dpi) {
    return dpi ? physicalPixels * 96.0 / (double)dpi : physicalPixels;
}

// How many item rows fit in a height already expressed in DIPs. Pitch is one
// item plus one gap; the trailing gap of the last row is not required, hence
// the + spacing.
//
// RESERVE FIRST. This is the height available to the ITEM GRID, not the whole
// taskbar. Anything else that occupies vertical space — outer padY, an extra
// item shaped as a row (a sliver above or below) — must be subtracted before
// calling, or the grid claims height that is already spoken for and the
// assembled group overflows its host.
inline int RowsInHeight(double heightDip, double itemHeight, double spacing) {
    double pitch = itemHeight + std::max(0.0, spacing);
    if (pitch <= 0.0 || heightDip <= 0.0)
        return 1;
    return std::max(1, (int)((heightDip + std::max(0.0, spacing)) / pitch));
}

// Convenience for the common case with nothing else reserved.
inline int AvailableRows(double taskbarHeightPx, unsigned dpi,
                         double itemHeight, double spacing) {
    return RowsInHeight(PixelsToDip(taskbarHeightPx, dpi), itemHeight, spacing);
}

// ---- The auto shape ---------------------------------------------------------
//
// Deterministic, not scored. Take the smallest column count reachable within
// the available rows — that is what "use the taskbar's height" means — and
// among the row counts that produce it, the one with the fewest empty slots.
// So 4 items with 3 rows available gives 2x2 rather than a ragged 3+1, and 5
// items with 4 rows available gives 3x2 rather than 4+1.

struct Shape {
    int rows = 1;
    int columns = 1;
};

inline Shape ChooseShape(int count, int maxRows) {
    if (count <= 0)
        return {0, 0};
    int limit = std::max(1, std::min(maxRows, count));
    Shape best{1, count};
    int bestWaste = 0;
    bool first = true;
    for (int rows = 1; rows <= limit; ++rows) {
        int columns = (count + rows - 1) / rows;
        int waste = rows * columns - count;
        if (first || columns < best.columns ||
            (columns == best.columns && waste < bestWaste)) {
            first = false;
            best = {rows, columns};
            bestWaste = waste;
        }
    }
    return best;
}

// ---- Expression generation --------------------------------------------------
//
// Turn a rows x columns shape into an expression so the auto path and the
// manual path are the same code below this point. Positions fill row-major
// (left to right, then down) for FillOrder::Rows or column-major (top to
// bottom, then right) for FillOrder::Columns. Grid positions past `count` are
// simply absent, so a ragged final row or column yields fewer tokens and the
// result is always a valid expression. Justify aligns that ragged group.
//
// Tokens come from namer(index); the default names items by 1-based number,
// matching what a user reads on screen. The caller's SizeResolver must map
// those same names back to pixel sizes.

using TokenNamer = std::function<std::wstring(int index)>;

inline std::wstring BuildGridExpression(int count, int rows, int columns,
                                        FillOrder fill,
                                        TokenNamer const& namer = {}) {
    if (count <= 0 || rows <= 0 || columns <= 0)
        return {};

    auto name = [&](int index) -> std::wstring {
        return namer ? namer(index) : std::to_wstring(index + 1);
    };

    // '|' groups are columns, ',' units are rows, always.
    std::wstring expr;
    for (int column = 0; column < columns; ++column) {
        std::wstring stack;
        for (int row = 0; row < rows; ++row) {
            int index = fill == FillOrder::Rows ? row * columns + column
                                                : column * rows + row;
            if (index < 0 || index >= count)
                continue;
            if (!stack.empty())
                stack += L", ";
            stack += name(index);
        }
        if (stack.empty())
            continue;
        if (!expr.empty())
            expr += L" | ";
        expr += stack;
    }
    return expr;
}

inline std::wstring BuildAutoExpression(int count, int maxRows, FillOrder fill,
                                        TokenNamer const& namer = {}) {
    Shape shape = ChooseShape(count, maxRows);
    return BuildGridExpression(count, shape.rows, shape.columns, fill, namer);
}

// ---- Items the arrangement forgot -------------------------------------------
//
// A hand-written arrangement names the items that existed when it was written.
// When the set is dynamic — a desktop is added, a folder appears — the new item
// is in no group, resolves to nothing, and silently vanishes from the taskbar.
// That is a trap, so a mod with a dynamic set offers a policy:
//
//   Append (default) — arrange the unlisted items automatically and put that
//                      block after everything the user wrote, so a new item is
//                      always reachable and the written block stays intact.
//   Ignore           — the arrangement is the whole truth; unlisted items stay
//                      off the taskbar until the user adds them.
//
// A mod that appends should log that it did, so the user knows to fold the new
// item into their arrangement when they next edit it.

// Whether a token the user wrote refers to the same item as one the mod
// expects. Defaults to a case-insensitive name match, which is WRONG for any
// mod that accepts aliases: "desktop1" and "1" are the same button, and
// comparing them as strings makes every aliased item look missing and get
// appended a second time. A mod with a vocabulary must supply this.
using TokenMatcher =
    std::function<bool(std::wstring const& placed, std::wstring const& expected)>;

inline std::vector<std::wstring> MissingTokens(
    std::vector<std::wstring> const& expected,
    std::vector<Placement> const& placements,
    TokenMatcher const& same = {}) {
    std::vector<std::wstring> missing;
    for (auto const& token : expected) {
        bool found = false;
        for (auto const& placement : placements) {
            bool match = same ? same(placement.token, token)
                              : TokenIs(placement.token, token.c_str());
            if (match) {
                found = true;
                break;
            }
        }
        if (!found)
            missing.push_back(token);
    }
    return missing;
}

inline std::wstring AppendMissing(std::wstring const& expression,
                                  std::vector<std::wstring> const& missing,
                                  int maxRows, FillOrder fill) {
    if (missing.empty())
        return expression;
    auto namer = [&missing](int index) { return missing[index]; };
    std::wstring block = BuildAutoExpression((int)missing.size(), maxRows, fill,
                                             namer);
    if (block.empty())
        return expression;
    if (expression.empty())
        return block;
    return L"(" + expression + L") | (" + block + L")";
}

// ---- The one setting --------------------------------------------------------
//
// Resolve `Layout.Arrangement` to the expression to arrange. Empty or the word
// "auto" (any case, surrounding space ignored) means generate one. The caller
// logs the result when wasAuto is true so the user can paste it back into the
// same field and edit it.

struct Arrangement {
    std::wstring expression;
    bool wasAuto = false;
};

inline bool IsAutoSetting(std::wstring const& setting) {
    size_t first = setting.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos)
        return true;
    size_t last = setting.find_last_not_of(L" \t\r\n");
    std::wstring trimmed = setting.substr(first, last - first + 1);
    if (trimmed.size() != 4)
        return false;
    for (size_t i = 0; i < 4; ++i)
        if (towlower(trimmed[i]) != L"auto"[i])
            return false;
    return true;
}

inline Arrangement ResolveArrangement(std::wstring const& setting, int count,
                                      int maxRows, FillOrder fill,
                                      TokenNamer const& namer = {}) {
    if (IsAutoSetting(setting))
        return {BuildAutoExpression(count, maxRows, fill, namer), true};
    return {setting, false};
}

}  // namespace windhawk_mod_templates::nested_group_layout

namespace ngl = windhawk_mod_templates::nested_group_layout;

// ============================================================
// Injected SystemTrayFrameGrid column
// Template block: _templates/injected-grid-column.h v1.2 (verbatim copy —
// keep in sync with the template; Windhawk mods are single-file).
// ============================================================

namespace windhawk_mod_templates::injected_grid_column {

using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::UI::Xaml::GridUnitType;
using winrt::Windows::UI::Xaml::Controls::ColumnDefinition;
using winrt::Windows::UI::Xaml::Controls::Grid;

enum class Anchor {
    BeforeIcons,
    BeforeOmni,
    BeforeClock,
    AfterClock,
    AfterShowDesktop,
};

struct Lease {
    std::wstring markerName;
    int column = -1;
};

inline FrameworkElement FindDirectChild(Grid const& parent,
                                        wchar_t const* name) {
    for (auto const& child : parent.Children()) {
        auto element = child.try_as<FrameworkElement>();
        if (element && element.Name() == name)
            return element;
    }
    return nullptr;
}

inline bool ResolveColumn(Grid const& parent, Anchor anchor, int& column) {
    if (anchor == Anchor::BeforeIcons) {
        column = 0;
        return true;
    }

    wchar_t const* referenceName = nullptr;
    bool after = false;
    switch (anchor) {
        case Anchor::BeforeOmni:
            referenceName = L"ControlCenterButton";
            break;
        case Anchor::BeforeClock:
            referenceName = L"NotificationCenterButton";
            break;
        case Anchor::AfterClock:
            referenceName = L"ShowDesktopStack";
            break;
        case Anchor::AfterShowDesktop:
            referenceName = L"ShowDesktopStack";
            after = true;
            break;
        case Anchor::BeforeIcons:
            break;
    }

    auto reference = FindDirectChild(parent, referenceName);
    if (!reference)
        return false; // Never silently turn an unavailable anchor into column 0.
    column = Grid::GetColumn(reference) +
             (after ? std::max(1, Grid::GetColumnSpan(reference)) : 0);
    return true;
}

inline bool AcquireAt(Grid const& parent, int column,
                      std::wstring const& markerName, Lease& lease) {
    if (!parent || column < 0 || markerName.empty() ||
        FindDirectChild(parent, markerName.c_str()))
        return false;

    ColumnDefinition definition;
    definition.Width({1.0, GridUnitType::Auto});
    if (static_cast<uint32_t>(column) < parent.ColumnDefinitions().Size())
        parent.ColumnDefinitions().InsertAt(column, definition);
    else
        parent.ColumnDefinitions().Append(definition);

    for (auto const& child : parent.Children()) {
        auto element = child.try_as<FrameworkElement>();
        if (!element) continue;
        int start = Grid::GetColumn(element);
        int span = Grid::GetColumnSpan(element);
        if (start >= column)
            Grid::SetColumn(element, start + 1);
        else if (start + span > column)
            Grid::SetColumnSpan(element, span + 1);
    }

    Grid marker;
    marker.Name(markerName);
    marker.Width(0.0);
    marker.Height(0.0);
    marker.IsHitTestVisible(false);
    Grid::SetColumn(marker, column);
    parent.Children().Append(marker);

    lease = {markerName, column};
    return true;
}

inline bool Acquire(Grid const& parent, Anchor anchor,
                    std::wstring const& markerName, Lease& lease) {
    int column = -1;
    if (!parent || !ResolveColumn(parent, anchor, column))
        return false;
    return AcquireAt(parent, column, markerName, lease);
}

inline bool Release(Grid const& parent, Lease& lease) {
    if (!parent || lease.markerName.empty())
        return false;

    uint32_t markerIndex = 0;
    bool found = false;
    int liveColumn = lease.column;
    for (uint32_t i = 0; i < parent.Children().Size(); ++i) {
        auto element = parent.Children().GetAt(i).try_as<FrameworkElement>();
        if (element && element.Name() == lease.markerName) {
            markerIndex = i;
            liveColumn = Grid::GetColumn(element);
            found = true;
            break;
        }
    }
    if (!found || liveColumn < 0)
        return false;

    parent.Children().RemoveAt(markerIndex);
    if (static_cast<uint32_t>(liveColumn) < parent.ColumnDefinitions().Size())
        parent.ColumnDefinitions().RemoveAt(liveColumn);

    for (auto const& child : parent.Children()) {
        auto element = child.try_as<FrameworkElement>();
        if (!element) continue;
        int start = Grid::GetColumn(element);
        int span = Grid::GetColumnSpan(element);
        if (start > liveColumn)
            Grid::SetColumn(element, start - 1);
        else if (start < liveColumn && start + span > liveColumn)
            Grid::SetColumnSpan(element, std::max(1, span - 1));
    }

    lease = {};
    return true;
}

} // namespace windhawk_mod_templates::injected_grid_column

namespace lease_column = windhawk_mod_templates::injected_grid_column;

// ============================================================
// Start-adjacent owned group
// Template block: _templates/start-placement.h v1.1 (verbatim copy — keep in
// sync with the template; Windhawk mods are single-file).
// ============================================================

namespace windhawk_mod_templates::start_placement {

using winrt::Windows::UI::Xaml::DependencyObject;
using winrt::Windows::UI::Xaml::FrameworkElement;
using winrt::Windows::UI::Xaml::HorizontalAlignment;
using winrt::Windows::UI::Xaml::Thickness;
using winrt::Windows::UI::Xaml::UIElement;
using winrt::Windows::UI::Xaml::VerticalAlignment;
using winrt::Windows::UI::Xaml::Visibility;
using winrt::Windows::UI::Xaml::Automation::AutomationProperties;
using winrt::Windows::UI::Xaml::Controls::Canvas;
using winrt::Windows::UI::Xaml::Controls::Grid;
using winrt::Windows::UI::Xaml::Media::TranslateTransform;
using winrt::Windows::UI::Xaml::Media::VisualTreeHelper;

enum class Side {
    Left,
    Right,
};

struct Lease {
    Grid group{nullptr};
    Grid rootGrid{nullptr};
    FrameworkElement startButton{nullptr};
    FrameworkElement taskItemsPanel{nullptr};
    Thickness groupOriginalMargin{};
    Thickness taskItemsPanelOriginalMargin{};
    bool startInTaskItemsPanel = false;
    winrt::event_token layoutToken{};
    Side side = Side::Left;
    double spacing = 0.0;
};

template<typename Predicate>
inline FrameworkElement FindDescendant(FrameworkElement const& root,
                                       Predicate&& predicate,
                                       int depth = 0) {
    if (!root || depth > 64)
        return nullptr;
    if (predicate(root))
        return root;
    int count = VisualTreeHelper::GetChildrenCount(root);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(root, i)
                         .try_as<FrameworkElement>();
        auto match = FindDescendant(
            child, std::forward<Predicate>(predicate), depth + 1);
        if (match)
            return match;
    }
    return nullptr;
}

inline Grid FindTaskbarRootGrid(FrameworkElement const& root) {
    auto taskbarFrame = FindDescendant(
        root, [](FrameworkElement const& element) {
            return winrt::get_class_name(element) ==
                   L"Taskbar.TaskbarFrame";
        });
    if (!taskbarFrame)
        return nullptr;

    int count = VisualTreeHelper::GetChildrenCount(taskbarFrame);
    for (int i = 0; i < count; ++i) {
        auto child = VisualTreeHelper::GetChild(taskbarFrame, i)
                         .try_as<Grid>();
        if (child && child.Name() == L"RootGrid")
            return child;
    }
    return nullptr;
}

inline FrameworkElement FindStartButton(FrameworkElement const& root) {
    return FindDescendant(
        root, [](FrameworkElement const& element) {
            return winrt::get_class_name(element) ==
                       L"Taskbar.ExperienceToggleButton" &&
                   AutomationProperties::GetAutomationId(element) ==
                       L"StartButton";
        });
}

inline bool Position(Lease& lease) noexcept {
    if (!lease.group || !lease.rootGrid || !lease.startButton)
        return false;

    try {
        double groupWidth = lease.group.Width() +
                            lease.groupOriginalMargin.Left +
                            lease.groupOriginalMargin.Right;
        double groupHeight = lease.group.Height() +
                             lease.groupOriginalMargin.Top +
                             lease.groupOriginalMargin.Bottom;
        bool startHidden =
            lease.startButton.Visibility() == Visibility::Collapsed;
        double startWidth = lease.startButton.ActualWidth();
        double startHeight = lease.startButton.ActualHeight();
        if (startWidth <= 0.0 && !startHidden)
            startWidth = 44.0;
        if (startHeight <= 0.0)
            startHeight = groupHeight;

        // rawX is Start's live layout position with our own counter-shift
        // backed out. It is re-read on every layout pass, so task-list churn
        // on a center-aligned taskbar re-centers the group naturally.
        auto transform = lease.startButton.TransformToVisual(lease.rootGrid);
        auto point = transform.TransformPoint({0.0f, 0.0f});
        auto existingShift =
            lease.startButton.RenderTransform().try_as<TranslateTransform>();
        double currentShift = existingShift ? existingShift.X() : 0.0;
        double rawX = point.X - currentShift;

        double spacing = std::max(0.0, lease.spacing);
        double push = groupWidth + spacing;
        if (lease.taskItemsPanel) {
            auto margin = lease.taskItemsPanel.Margin();
            double needed =
                lease.taskItemsPanelOriginalMargin.Left + push;
            if (std::fabs(margin.Left - needed) > 0.5) {
                margin.Left = needed;
                lease.taskItemsPanel.Margin(margin);
            }
        }

        // The Start counter-shift is a constant per mode, not an absolute-
        // anchor correction. When Start rides the repeater-margin push, room
        // for a Left group already opens at the block's left edge (no shift),
        // and a Right group needs Start pulled back so the gap opens between
        // Start and the task items. When Start sits outside the repeater the
        // roles invert: the pushed items leave the Right gap by themselves,
        // and a Left group needs Start pushed out of the way instead.
        double neededShift;
        if (lease.side == Side::Left)
            neededShift = lease.startInTaskItemsPanel ? 0.0 : push;
        else
            neededShift = lease.startInTaskItemsPanel ? -push : 0.0;
        if (startHidden)
            neededShift = 0.0;

        if (std::fabs(neededShift) <= 0.5) {
            if (existingShift || lease.startButton.RenderTransform())
                lease.startButton.ClearValue(
                    UIElement::RenderTransformProperty());
        } else if (std::fabs(currentShift - neededShift) > 0.5) {
            TranslateTransform startShift;
            startShift.X(neededShift);
            lease.startButton.RenderTransform(startShift);
        }

        // Place the group relative to where Start actually ends up.
        double startFinalX = rawX + neededShift;
        double left = lease.side == Side::Left
                          ? startFinalX - groupWidth - spacing
                          : startFinalX + startWidth + spacing;
        if (left < 0.0)
            left = 0.0;

        double top = point.Y + (startHeight - groupHeight) / 2.0;
        if (top < 0.0)
            top = 0.0;
        double rootWidth = lease.rootGrid.ActualWidth();
        if (rootWidth > 0.0 && left + groupWidth > rootWidth)
            left = std::max(0.0, rootWidth - groupWidth);

        auto target = lease.groupOriginalMargin;
        target.Left += left;
        target.Top += top;
        auto current = lease.group.Margin();
        if (std::fabs(current.Left - target.Left) > 0.5 ||
            std::fabs(current.Top - target.Top) > 0.5) {
            lease.group.Margin(target);
        }
        return true;
    } catch (...) {
        return false;
    }
}

inline bool Release(Lease& lease) noexcept {
    if (!lease.group)
        return false;

    try {
        if (lease.rootGrid && lease.layoutToken)
            lease.rootGrid.LayoutUpdated(lease.layoutToken);
        if (lease.taskItemsPanel)
            lease.taskItemsPanel.Margin(
                lease.taskItemsPanelOriginalMargin);
        if (lease.startButton)
            lease.startButton.ClearValue(
                UIElement::RenderTransformProperty());
        lease.group.Margin(lease.groupOriginalMargin);
        if (lease.rootGrid) {
            uint32_t index = 0;
            if (lease.rootGrid.Children().IndexOf(lease.group, index))
                lease.rootGrid.Children().RemoveAt(index);
        }
    } catch (...) {
        lease = {};
        return false;
    }
    lease = {};
    return true;
}

inline bool Acquire(FrameworkElement const& root, Grid const& group,
                    Side side, double spacing, Lease& lease) {
    if (!root || !group || lease.group || group.Width() <= 0.0 ||
        group.Height() <= 0.0)
        return false;

    auto rootGrid = FindTaskbarRootGrid(root);
    auto startButton = FindStartButton(root);
    if (!rootGrid || !startButton)
        return false;

    lease.group = group;
    lease.rootGrid = rootGrid;
    lease.startButton = startButton;
    lease.groupOriginalMargin = group.Margin();
    lease.side = side;
    lease.spacing = spacing;

    group.HorizontalAlignment(HorizontalAlignment::Left);
    group.VerticalAlignment(VerticalAlignment::Top);
    Grid::SetColumn(group, 0);
    Grid::SetColumnSpan(
        group,
        std::max(1, static_cast<int>(
                        rootGrid.ColumnDefinitions().Size())));
    Canvas::SetZIndex(group, 1000);
    rootGrid.Children().Append(group);

    lease.taskItemsPanel = FindDescendant(
        rootGrid, [](FrameworkElement const& element) {
            return element.Name() == L"TaskbarFrameRepeater";
        });
    if (lease.taskItemsPanel) {
        lease.taskItemsPanelOriginalMargin =
            lease.taskItemsPanel.Margin();
        // Whether Start rides the repeater-margin push is build-dependent.
        // Resolve it from the visual tree instead of inferring from motion.
        try {
            auto panel = lease.taskItemsPanel.as<DependencyObject>();
            for (auto node = startButton.as<DependencyObject>(); node;
                 node = VisualTreeHelper::GetParent(node)) {
                if (node == panel) {
                    lease.startInTaskItemsPanel = true;
                    break;
                }
            }
        } catch (...) {
            lease.startInTaskItemsPanel = false;
        }
    }

    if (!Position(lease)) {
        Release(lease);
        return false;
    }
    lease.layoutToken = rootGrid.LayoutUpdated(
        [&lease](auto const&, auto const&) {
            Position(lease);
        });
    return true;
}

} // namespace windhawk_mod_templates::start_placement

namespace start_placement = windhawk_mod_templates::start_placement;

// ============================================================
// Settings
// ============================================================

struct ModSettings {
    // Placement
    std::wstring position = L"beforeOmni";
    // Content
    bool location = true;
    bool microphone = true;
    bool camera = true;
    bool copilot = true;
    // Layout
    std::wstring arrangement = L"auto";
    std::wstring fillOrder = L"rows";
    std::wstring justify = L"center";
    std::wstring newItems = L"append";
    // Size
    int itemSize = 16;
    int itemSpacing = 4;
    // Adjust
    int padX = 0;
    int padY = 0;
    int offsetX = 0;
    int offsetY = 0;
    // Surface: canonical icon-surface fields first.
    int idleOpacity = 50;
    int activeOpacity = 100;
    bool glowEnabled = false;
    int glowOpacity = 40;
    bool slashColorSet = false;
    winrt::Windows::UI::Color slashColorValue{};
    std::wstring slashDirection = L"falling";
    int slashOpacity = 100;
    // Surface: Privacy Anchor extensions.
    bool idleColorSet = false;
    winrt::Windows::UI::Color idleColorValue{};
    bool activeColorSet = false;
    winrt::Windows::UI::Color activeColorValue{};
    int disabledOpacity = 50;
    bool disabledColorSet = false;
    winrt::Windows::UI::Color disabledColorValue{};
    bool alertWhenBlockedAndActive = true;
    std::wstring glowStyle = L"radiate";
    bool glowColorSet = false;
    winrt::Windows::UI::Color glowColorValue{};
    int glowSize = 220;
    int glowSpeed = 1200;
    // Behavior
    bool cameraHardwareDetection = false;
    bool suppressNativeIndicators = true;
};
static ModSettings g_settings;  // exit-time-safe: heap-only
static std::atomic<bool> g_cameraHardwareDetectionEnabled{false};
static std::atomic<bool> g_cameraItemEnabled{true};
static std::atomic<bool> g_copilotItemEnabled{true};

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
    auto Bool = [](PCWSTR name) { return Wh_GetIntSetting(name) != 0; };

    g_settings.position = GetStringSetting(L"Placement.Position");

    g_settings.location = Bool(L"Content.Location");
    g_settings.microphone = Bool(L"Content.Microphone");
    g_settings.camera = Bool(L"Content.Camera");
    g_settings.copilot = Bool(L"Content.Copilot");

    g_settings.arrangement = GetStringSetting(L"Layout.Arrangement");
    g_settings.fillOrder = GetStringSetting(L"Layout.FillOrder");
    g_settings.justify = GetStringSetting(L"Layout.Justify");
    g_settings.newItems = GetStringSetting(L"Layout.NewItems");

    g_settings.itemSize = clamp(Wh_GetIntSetting(L"Size.ItemSize"), 8, 48);
    g_settings.itemSpacing = clamp(Wh_GetIntSetting(L"Size.ItemSpacing"), 0, 40);

    g_settings.padX = clamp(Wh_GetIntSetting(L"Adjust.PadX"), 0, 40);
    g_settings.padY = clamp(Wh_GetIntSetting(L"Adjust.PadY"), 0, 40);
    g_settings.offsetX = clamp(Wh_GetIntSetting(L"Adjust.OffsetX"), -40, 40);
    g_settings.offsetY = clamp(Wh_GetIntSetting(L"Adjust.OffsetY"), -40, 40);

    g_settings.idleOpacity = clamp(Wh_GetIntSetting(L"Surface.IdleOpacity"), 0, 100);
    g_settings.activeOpacity = clamp(Wh_GetIntSetting(L"Surface.ActiveOpacity"), 0, 100);
    g_settings.glowEnabled = Bool(L"Surface.GlowEnabled");
    g_settings.glowOpacity = clamp(Wh_GetIntSetting(L"Surface.GlowOpacity"), 0, 100);
    g_settings.slashColorSet = ParseColorToken(
        GetStringSetting(L"Surface.SlashColor").c_str(), g_settings.slashColorValue);
    g_settings.slashDirection = GetStringSetting(L"Surface.SlashDirection");
    g_settings.slashOpacity = clamp(Wh_GetIntSetting(L"Surface.SlashOpacity"), 0, 100);
    g_settings.idleColorSet = ParseColorToken(
        GetStringSetting(L"Surface.IdleColor").c_str(), g_settings.idleColorValue);
    g_settings.activeColorSet = ParseColorToken(
        GetStringSetting(L"Surface.ActiveColor").c_str(), g_settings.activeColorValue);
    g_settings.disabledOpacity = clamp(Wh_GetIntSetting(L"Surface.DisabledOpacity"), 0, 100);
    g_settings.disabledColorSet = ParseColorToken(
        GetStringSetting(L"Surface.DisabledColor").c_str(), g_settings.disabledColorValue);
    g_settings.alertWhenBlockedAndActive = Bool(L"Surface.AlertWhenBlockedAndActive");
    g_settings.glowStyle = GetStringSetting(L"Surface.GlowStyle");
    if (g_settings.glowStyle != L"steady" && g_settings.glowStyle != L"pulse")
        g_settings.glowStyle = L"radiate";
    g_settings.glowColorSet = ParseColorToken(
        GetStringSetting(L"Surface.GlowColor").c_str(), g_settings.glowColorValue);
    g_settings.glowSize = clamp(Wh_GetIntSetting(L"Surface.GlowSize"), 100, 300);
    g_settings.glowSpeed = clamp(Wh_GetIntSetting(L"Surface.GlowSpeed"), 250, 5000);

    g_settings.cameraHardwareDetection =
        Bool(L"Behavior.CameraHardwareDetection");
    g_settings.suppressNativeIndicators =
        Bool(L"Behavior.SuppressNativeIndicators");
    g_cameraHardwareDetectionEnabled.store(
        g_settings.cameraHardwareDetection);
    g_cameraItemEnabled.store(g_settings.camera);
    g_copilotItemEnabled.store(g_settings.copilot);
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
static std::atomic<bool>  g_taskbarRestarted{false};
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
static std::atomic<bool> g_taskbarDarkTheme{true};
// Explorer process shutdown doesn't guarantee a Wh_ModUninit call. Keep all
// namespace-scope XAML/WinRT owners out of CRT global destruction so they
// can't release taskbar objects after XAML has torn down or from the shutdown
// thread. Controlled unload still clears them explicitly on the taskbar UI
// thread in Wh_ModUninit.
[[clang::no_destroy]] static Grid g_syntheticGrid = nullptr;
[[clang::no_destroy]] static FrameworkElement g_locIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_micIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_camIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_copilotIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_locSlot = nullptr;
[[clang::no_destroy]] static FrameworkElement g_micSlot = nullptr;
[[clang::no_destroy]] static FrameworkElement g_camSlot = nullptr;
[[clang::no_destroy]] static FrameworkElement g_copilotSlot = nullptr;
[[clang::no_destroy]] static FrameworkElement g_locGlowIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_micGlowIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_camGlowIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_copilotGlowIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_locSlashIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_micSlashIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_camSlashIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_copilotSlashIcon = nullptr;
[[clang::no_destroy]] static FrameworkElement g_syntheticParent = nullptr;
static lease_column::Lease g_columnLease;  // exit-time-safe: heap-only
[[clang::no_destroy]] static start_placement::Lease g_startLease;

struct SlotEventState {
    FrameworkElement element{nullptr};
    winrt::event_token tappedToken{};
};
[[clang::no_destroy]] static std::optional<std::vector<SlotEventState>>
    g_slotEventStates{std::in_place};

struct GlowAnimationState {
    FrameworkElement element{nullptr};
    std::vector<winrt::Windows::UI::Xaml::Media::Animation::Storyboard>
        storyboards;
    bool running = false;
};
[[clang::no_destroy]] static std::optional<std::vector<GlowAnimationState>>
    g_glowAnimationStates{std::in_place};

struct PrivacyState {
    enum class Type { Location, Mic, Camera, Both };
    winrt::weak_ref<FrameworkElement> iconViewRef;
    winrt::weak_ref<TextBlock>        textBlockRef;
    int64_t textToken       = 0;
    int64_t visibilityToken = 0;
    Type    type      = Type::Location;
};
// PrivacyState holds only winrt::weak_ref and integers; weak_ref release is a
// thread-safe in-process refcount decrement, so the normal destructor is
// leak-free (rule 1). See lifecycle template v1.3.1.
static std::vector<PrivacyState> g_privacyStates;  // exit-time-safe: heap-only

using FrameworkElementLoadedRevoker = winrt::impl::event_revoker<
    IFrameworkElement,
    &winrt::impl::abi<IFrameworkElement>::type::remove_Loaded>;
[[clang::no_destroy]] static std::optional<std::list<FrameworkElementLoadedRevoker>>
    g_loadedRevokers{std::in_place};

// Forward declarations
static bool ApplyStyle();
static bool ApplyOnTaskbarThread();
static void ApplyStyleOnWindowThread();
static void ClearPrivacyStates();
static void RemoveSyntheticIcons();
static void RemoveModUi();
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
// Template block: _templates/taskbar-xaml-lifecycle.template.cpp v1.2.
// Taskbar discovery, XAML-root access, window-thread dispatch, symbol hooks,
// taskbar restart trigger, and unload rules stay verbatim where the
// privacy-specific monitoring worker doesn't require an adapter.
// ============================================================

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void*, void*);
static CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;
using TaskbarHost_FrameHeight_t = int (WINAPI*)(void*);
static TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;
using std__Ref_count_base__Decref_t = void (WINAPI*)(void*);
static std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;
static void* CTaskBand_ITaskListWndSite_vftable;

static XamlRoot GetTaskbarXamlRoot(HWND taskbarWindow) {
    if (!CTaskBand_GetTaskbarHost_Original ||
        !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable)
        return nullptr;

    HWND taskSwitchWindow = reinterpret_cast<HWND>(
        GetPropW(taskbarWindow, L"TaskbandHWND"));
    if (!taskSwitchWindow) return nullptr;
    void* taskBand = reinterpret_cast<void*>(
        GetWindowLongPtrW(taskSwitchWindow, 0));
    if (!taskBand) return nullptr;

    void* taskBandForSite = taskBand;
    for (int i = 0;
         *reinterpret_cast<void**>(taskBandForSite) !=
             CTaskBand_ITaskListWndSite_vftable;
         ++i) {
        if (i == 20) return nullptr;
        taskBandForSite = reinterpret_cast<void**>(taskBandForSite) + 1;
    }

    void* taskbarHostSharedPtr[2]{};
    CTaskBand_GetTaskbarHost_Original(taskBandForSite,
                                     taskbarHostSharedPtr);
    if (!taskbarHostSharedPtr[0] || !taskbarHostSharedPtr[1]) {
        if (taskbarHostSharedPtr[1])
            std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    size_t elementOffset = 0x10;
#if defined(_M_X64)
    auto bytes = reinterpret_cast<BYTE const*>(TaskbarHost_FrameHeight_Original);
    if (bytes[0] == 0x48 && bytes[1] == 0x83 && bytes[2] == 0xEC &&
        bytes[4] == 0x48 && bytes[5] == 0x83 && bytes[6] == 0xC1 &&
        bytes[7] <= 0x7F) {
        elementOffset = bytes[7];
    } else {
        Wh_Log(L"[Template] Unsupported TaskbarHost::FrameHeight");
    }
#elif defined(_M_ARM64)
    auto instructions = reinterpret_cast<DWORD const*>(
        TaskbarHost_FrameHeight_Original);
    if (instructions[0] == 0xD503237F &&
        (instructions[1] & 0xFFC07FFF) == 0xA9807BFD &&
        instructions[2] == 0x910003FD &&
        (instructions[3] & 0xFFF00FE0) == 0xF8400C00) {
        elementOffset = (instructions[3] >> 12) & 0xFF;
    } else {
        Wh_Log(L"[Template] Unsupported TaskbarHost::FrameHeight");
    }
#else
#error Unsupported architecture
#endif

    auto unknown = *reinterpret_cast<IUnknown**>(
        reinterpret_cast<BYTE*>(taskbarHostSharedPtr[0]) + elementOffset);
    if (!unknown) {
        std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
        return nullptr;
    }

    FrameworkElement taskbarElement = nullptr;
    unknown->QueryInterface(winrt::guid_of<FrameworkElement>(),
                            winrt::put_abi(taskbarElement));
    auto result = taskbarElement ? taskbarElement.XamlRoot() : nullptr;
    std__Ref_count_base__Decref_Original(taskbarHostSharedPtr[1]);
    return result;
}

// ============================================================
// Window thread marshalling / taskbar discovery
// ============================================================

using WindowThreadProc = void (*)(void*);

static void LogCurrentUiException(PCWSTR context) noexcept {
    try {
        throw;
    } catch (winrt::hresult_error const& error) {
        Wh_Log(L"[Lifecycle] %s failed hr=0x%08X: %s", context,
               static_cast<unsigned>(error.code().value),
               error.message().c_str());
    } catch (std::exception const&) {
        Wh_Log(L"[Lifecycle] %s failed with a C++ exception", context);
    } catch (...) {
        Wh_Log(L"[Lifecycle] %s failed with an unknown exception", context);
    }
}

static bool InvokeWindowThreadProc(WindowThreadProc proc, void* parameter) {
    try {
        proc(parameter);
        return true;
    } catch (...) {
        LogCurrentUiException(L"UI callback");
    }
    return false;
}

static bool RunFromWindowThread(HWND window, WindowThreadProc proc,
                                void* parameter) {
    static UINT message = RegisterWindowMessageW(
        L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Dispatch {
        WindowThreadProc proc;
        void* parameter;
        bool succeeded = false;
    };
    DWORD threadId = GetWindowThreadProcessId(window, nullptr);
    if (!threadId) return false;
    if (threadId == GetCurrentThreadId()) {
        return InvokeWindowThreadProc(proc, parameter);
    }

    HHOOK hook = SetWindowsHookExW(
        WH_CALLWNDPROC,
        [](int code, WPARAM wParam, LPARAM lParam) -> LRESULT {
            if (code == HC_ACTION) {
                auto call = reinterpret_cast<CWPSTRUCT const*>(lParam);
                static UINT dispatchMessage = RegisterWindowMessageW(
                    L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
                if (call->message == dispatchMessage) {
                    auto dispatch = reinterpret_cast<Dispatch*>(call->lParam);
                    dispatch->succeeded = InvokeWindowThreadProc(
                        dispatch->proc, dispatch->parameter);
                }
            }
            return CallNextHookEx(nullptr, code, wParam, lParam);
        },
        nullptr, threadId);
    if (!hook) return false;

    Dispatch dispatch{proc, parameter};
    SendMessageW(window, message, 0, reinterpret_cast<LPARAM>(&dispatch));
    UnhookWindowsHookEx(hook);
    return dispatch.succeeded;
}

static HWND FindCurrentProcessTaskbarWnd() {
    HWND result = nullptr;
    EnumWindows([](HWND window, LPARAM parameter) -> BOOL {
        DWORD processId = 0;
        wchar_t className[32];
        if (GetWindowThreadProcessId(window, &processId) &&
            processId == GetCurrentProcessId() &&
            GetClassNameW(window, className, ARRAYSIZE(className)) &&
            _wcsicmp(className, L"Shell_TrayWnd") == 0) {
            *reinterpret_cast<HWND*>(parameter) = window;
            return FALSE;
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
// Unified icon layout
// ============================================================

static bool IsMicrophoneToken(std::wstring const& token) {
    return ngl::TokenIs(token, L"mic") ||
           ngl::TokenIs(token, L"microphone");
}

static bool TryResolvePrivacyToken(std::wstring const& token,
                                   PrivacyItemKind& kind) {
    if (ngl::TokenIs(token, L"location")) {
        kind = PrivacyItemKind::Location;
        return true;
    }
    if (IsMicrophoneToken(token)) {
        kind = PrivacyItemKind::Microphone;
        return true;
    }
    if (ngl::TokenIs(token, L"camera")) {
        kind = PrivacyItemKind::Camera;
        return true;
    }
    if (ngl::TokenIs(token, L"copilot")) {
        kind = PrivacyItemKind::Copilot;
        return true;
    }
    return false;
}

static bool PrivacyItemEnabled(PrivacyItemKind kind) {
    switch (kind) {
        case PrivacyItemKind::Location: return g_settings.location;
        case PrivacyItemKind::Microphone: return g_settings.microphone;
        case PrivacyItemKind::Camera: return g_settings.camera;
        case PrivacyItemKind::Copilot: return g_settings.copilot;
    }
    return false;
}

static std::vector<std::wstring> EnabledPrivacyTokens() {
    std::vector<std::wstring> tokens;
    if (g_settings.location) tokens.push_back(L"location");
    if (g_settings.microphone) tokens.push_back(L"mic");
    if (g_settings.camera) tokens.push_back(L"camera");
    if (g_settings.copilot) tokens.push_back(L"copilot");
    return tokens;
}

static bool SamePrivacyItem(std::wstring const& placed,
                            std::wstring const& expected) {
    PrivacyItemKind placedKind;
    PrivacyItemKind expectedKind;
    return TryResolvePrivacyToken(placed, placedKind) &&
           TryResolvePrivacyToken(expected, expectedKind) &&
           placedKind == expectedKind;
}

static ngl::Size ResolvePrivacyLayoutToken(std::wstring const& token) {
    PrivacyItemKind kind;
    if (!TryResolvePrivacyToken(token, kind) || !PrivacyItemEnabled(kind))
        return {};
    return {(double)g_settings.itemSize, (double)g_settings.itemSize};
}

static ngl::FillOrder PrivacyFillOrder() {
    return g_settings.fillOrder == L"columns" ? ngl::FillOrder::Columns
                                               : ngl::FillOrder::Rows;
}

static ngl::Config PrivacyLayoutConfig() {
    ngl::Config config;
    config.spacing = (double)g_settings.itemSpacing;
    config.justify = g_settings.justify == L"start" ? ngl::Justify::Start
                   : g_settings.justify == L"end"   ? ngl::Justify::End
                                                     : ngl::Justify::Center;
    config.padX = (double)g_settings.padX;
    config.padY = (double)g_settings.padY;
    return config;
}

static int AvailablePrivacyRows() {
    HWND window = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    RECT rect{};
    if (!window || !GetWindowRect(window, &rect)) {
        Wh_Log(L"[Layout] No taskbar window - assuming a single row");
        return 1;
    }

    UINT dpi = GetDpiForWindow(window);
    double heightDip = ngl::PixelsToDip(
        (double)(rect.bottom - rect.top), dpi);
    double reserved = 2.0 * (double)g_settings.padY;
    int rows = ngl::RowsInHeight(
        heightDip - reserved, (double)g_settings.itemSize,
        (double)g_settings.itemSpacing);
    Wh_Log(L"[Layout] taskbar %dpx at %udpi = %.0f dip, %.0f reserved "
           L"-> %d row(s) for privacy icons",
           (int)(rect.bottom - rect.top), dpi, heightDip, reserved, rows);
    return rows;
}

static bool ComputePrivacyPlacements(
    std::vector<std::wstring> const& enabledTokens,
    std::vector<ngl::Placement>& placements, ngl::Size& total) {
    int maxRows = AvailablePrivacyRows();
    auto fill = PrivacyFillOrder();
    auto namer = [&enabledTokens](int index) {
        return enabledTokens[index];
    };
    auto arrangement = ngl::ResolveArrangement(
        g_settings.arrangement, (int)enabledTokens.size(), maxRows, fill,
        namer);
    std::wstring expression = arrangement.expression;
    ngl::ParseError error;
    bool ok = ngl::Compute(expression, PrivacyLayoutConfig(),
                           ResolvePrivacyLayoutToken, placements, total,
                           &error);
    if (!ok) {
        Wh_Log(L"[Layout] Arrangement \"%ls\" - expected %ls at character %d; "
               L"using the automatic arrangement instead",
               expression.c_str(), error.expected.c_str(),
               (int)error.position + 1);
        arrangement = ngl::ResolveArrangement(
            L"auto", (int)enabledTokens.size(), maxRows, fill, namer);
        expression = arrangement.expression;
        ok = ngl::Compute(expression, PrivacyLayoutConfig(),
                          ResolvePrivacyLayoutToken, placements, total,
                          nullptr);
    }

    if (ok && !arrangement.wasAuto && g_settings.newItems != L"ignore") {
        auto missing = ngl::MissingTokens(enabledTokens, placements,
                                          SamePrivacyItem);
        if (!missing.empty()) {
            expression = ngl::AppendMissing(expression, missing, maxRows, fill);
            ok = ngl::Compute(expression, PrivacyLayoutConfig(),
                              ResolvePrivacyLayoutToken, placements, total,
                              nullptr);
            Wh_Log(L"[Layout] %d newly enabled icon(s) missing from your "
                   L"arrangement were added",
                   (int)missing.size());
        }
    }

    Wh_Log(L"[Layout] tokens: location=Location  mic=Microphone  "
           L"camera=Camera  copilot=Copilot");
    Wh_Log(L"[Layout] %d enabled icon(s), arrangement = \"%ls\"%ls, "
           L"size %.0fx%.0f",
           (int)enabledTokens.size(), expression.c_str(),
           arrangement.wasAuto
               ? L" (auto - paste this into Arrangement to edit it)"
               : L"",
           total.width, total.height);
    return ok;
}

// ============================================================
// Synthetic icon management
// ============================================================

static void SetGlowActive(FrameworkElement const& glow, bool active) {
    if (!glow) return;
    glow.Visibility(active ? Visibility::Visible : Visibility::Collapsed);
    for (auto& state : *g_glowAnimationStates) {
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
    if (!g_syntheticGrid) return;
    double idleOpacity = g_settings.idleOpacity / 100.0;
    double activeOpacity = g_settings.activeOpacity / 100.0;
    double disabledOpacity = g_settings.disabledOpacity / 100.0;
    bool isDark = g_taskbarDarkTheme.load();
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
            icon.Opacity(activeOpacity);
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
    if (!g_syntheticGrid) return;
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
    tb.FontSize((double)g_settings.itemSize);
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
    double iconSize = static_cast<double>(g_settings.itemSize);
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

    g_glowAnimationStates->push_back(std::move(animationState));
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
    try {
        auto theme = root.ActualTheme();
        if (theme == ElementTheme::Dark)
            g_taskbarDarkTheme.store(true);
        else if (theme == ElementTheme::Light)
            g_taskbarDarkTheme.store(false);
    } catch (...) {
        // Keep the dark-taskbar-safe default until a live root reports a theme.
    }

    auto gridElem = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!gridElem) { Wh_Log(L"[Inject] SystemTrayFrameGrid not found"); return false; }
    auto gridParent = gridElem.try_as<Grid>();
    if (!gridParent) { Wh_Log(L"[Inject] SystemTrayFrameGrid not a Grid"); return false; }

    // Idempotent check.
    if (FindChildRecursive(root, [](FrameworkElement const& element) {
            return element.Name() == L"PrivacyAnchorBar";
        }))
        return true;

    auto enabledTokens = EnabledPrivacyTokens();
    if (enabledTokens.empty()) {
        Wh_Log(L"[Inject] All privacy icons are disabled");
        return true;
    }

    std::vector<ngl::Placement> placements;
    ngl::Size total;
    if (!ComputePrivacyPlacements(enabledTokens, placements, total))
        return false;

    bool startPosition = g_settings.position == L"leftOfStart" ||
                         g_settings.position == L"rightOfStart";
    int insertCol = -1;
    if (!startPosition) {
        lease_column::Anchor anchor = lease_column::Anchor::BeforeIcons;
        if (g_settings.position == L"beforeOmni")
            anchor = lease_column::Anchor::BeforeOmni;
        else if (g_settings.position == L"beforeClock")
            anchor = lease_column::Anchor::BeforeClock;
        else if (g_settings.position == L"afterClock")
            anchor = lease_column::Anchor::AfterClock;
        else if (g_settings.position == L"afterShowDesktop")
            anchor = lease_column::Anchor::AfterShowDesktop;

        if (!lease_column::ResolveColumn(gridParent, anchor, insertCol)) {
            Wh_Log(L"[Inject] Position anchor unavailable: %s",
                   g_settings.position.c_str());
            return false;
        }
    }

    // ── Build the anchor bar ────────────────────────────────────
    Grid bar;
    bar.Name(L"PrivacyAnchorBar");
    bar.Width(total.width);
    bar.Height(total.height);
    bar.VerticalAlignment(VerticalAlignment::Center);
    bar.HorizontalAlignment(HorizontalAlignment::Center);
    // Adjust.OffsetX/Y is cosmetic and does not participate in measurement.
    ApplyOffset(bar, g_settings.offsetX, g_settings.offsetY);

    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locSlot = nullptr; g_micSlot = nullptr; g_camSlot = nullptr; g_copilotSlot = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;

    for (auto const& placement : placements) {
        const auto& token = placement.token;
        PrivacyItemKind itemKind;
        if (!TryResolvePrivacyToken(token, itemKind) ||
            !PrivacyItemEnabled(itemKind))
            continue;
        const wchar_t* glyph    = L"";
        const wchar_t* iconFont = L"Segoe MDL2 Assets";
        bool  isActive, isDisabled;
        PrivacyBlockReason blockReason;
        const wchar_t* label;
        const wchar_t* idleLabel     = L"Not requested";

        if (itemKind == PrivacyItemKind::Location) {
            glyph        = L"\xE37A";
            isActive     = g_locActive.load() || g_locUsage.load();
            isDisabled   = g_locDisabled.load();
            blockReason  = g_locBlockReason.load();
            label        = L"Location";
        } else if (itemKind == PrivacyItemKind::Microphone) {
            glyph        = L"\xE720";
            isActive     = g_micActive.load() || g_micUsage.load();
            isDisabled   = g_micDisabled.load();
            blockReason  = g_micBlockReason.load();
            label        = L"Microphone";
        } else if (itemKind == PrivacyItemKind::Camera) {
            glyph        = L"\xE722";
            isActive     = g_camActive.load() || g_camUsage.load();
            isDisabled   = g_camDisabled.load();
            blockReason  = g_camBlockReason.load();
            label        = L"Camera";
        } else {  // copilot
            isActive      = g_copilotActive.load();
            isDisabled    = g_copilotDisabled.load();
            blockReason   = g_copilotBlockReason.load();
            label         = L"Copilot";
            idleLabel     = L"Installed (not running)";
        }

        // Wrap glow + icon + slash overlay in a 1-cell Grid so they overlap (back to front)
        Grid slot;
        slot.Width(placement.size.width);
        slot.Height(placement.size.height);
        slot.HorizontalAlignment(HorizontalAlignment::Left);
        slot.VerticalAlignment(VerticalAlignment::Top);
        slot.Margin({placement.x, placement.y, 0.0, 0.0});

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

        if (itemKind == PrivacyItemKind::Copilot) {
            bool isDark = g_taskbarDarkTheme.load();
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

            auto ip = tryMakePath((double)g_settings.itemSize);
            if (!ip) ip = makeStar((double)g_settings.itemSize);
            iconFe = ip; slot.Children().Append(ip);
        } else {
            auto tb = MakeIconTextBlock(glyph);
            tb.FontFamily(FontFamily(iconFont));
            iconFe = tb;
            slot.Children().Append(tb);
        }

        // Slash overlay — diagonal line across the icon, direction from settings
        double sz = (double)g_settings.itemSize;
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
                bool isDark = g_taskbarDarkTheme.load();
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
        g_slotEventStates->push_back({slot, tappedToken});

        if (itemKind == PrivacyItemKind::Location) {
            g_locSlot = slot; g_locIcon = iconFe;
            g_locGlowIcon = glowFe; g_locSlashIcon = slashLine;
        } else if (itemKind == PrivacyItemKind::Microphone) {
            g_micSlot = slot; g_micIcon = iconFe;
            g_micGlowIcon = glowFe; g_micSlashIcon = slashLine;
        } else if (itemKind == PrivacyItemKind::Camera) {
            g_camSlot = slot; g_camIcon = iconFe;
            g_camGlowIcon = glowFe; g_camSlashIcon = slashLine;
        } else {
            g_copilotSlot = slot; g_copilotIcon = iconFe;
            g_copilotGlowIcon = glowFe; g_copilotSlashIcon = slashLine;
        }

        bar.Children().Append(slot);
    }

    if (startPosition) {
        auto side = g_settings.position == L"leftOfStart"
                        ? start_placement::Side::Left
                        : start_placement::Side::Right;
        if (!start_placement::Acquire(
                root, bar, side, g_settings.itemSpacing,
                g_startLease)) {
            Wh_Log(L"[Inject] Start anchor unavailable: %s",
                   g_settings.position.c_str());
            return false;
        }
        g_syntheticParent = g_startLease.rootGrid;
    } else {
        if (!lease_column::AcquireAt(
                gridParent, insertCol, L"PrivacyAnchorColumnMarker",
                g_columnLease)) {
            Wh_Log(L"[Inject] Failed to acquire tray column %d", insertCol);
            return false;
        }
        try {
            Grid::SetColumn(bar, g_columnLease.column);
            gridParent.Children().Append(bar);
        } catch (...) {
            lease_column::Release(gridParent, g_columnLease);
            throw;
        }
        g_syntheticParent = gridElem;
    }

    g_syntheticGrid   = bar;

    UpdateSyntheticState();
    Wh_Log(L"[Inject] PrivacyAnchorBar: %d icon(s), %.0fx%.0f",
           (int)placements.size(), total.width, total.height);
    return true;
}

static void RemoveSyntheticIcons() {
    // XAML defers removed-subtree teardown to a later UI tick, which can land
    // after the mod DLL unloads. The boxed tooltip and automation-name values on
    // the synthetic icons are implemented in this DLL, so release them before
    // removal (same crash class as folder-menus crash-on-disable).
    for (auto& state : *g_slotEventStates) {
        if (!state.element) continue;
        try { state.element.Tapped(state.tappedToken); } catch (...) {}
    }
    g_slotEventStates->clear();

    // Storyboards retain their animation targets. Stop and release them before
    // removing the XAML subtree so no callback can outlive the mod DLL.
    for (auto& state : *g_glowAnimationStates) {
        for (auto const& storyboard : state.storyboards) {
            try { storyboard.Stop(); } catch (...) {}
        }
        state.storyboards.clear();
        state.element = nullptr;
        state.running = false;
    }
    g_glowAnimationStates->clear();

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

    auto gridParent =
        g_syntheticParent ? g_syntheticParent.try_as<Grid>() : nullptr;
    if (g_startLease.group) {
        if (!start_placement::Release(g_startLease))
            Wh_Log(L"[Remove] Start placement lease was not live");
    } else if (gridParent) {
        for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
            auto fe = gridParent.Children().GetAt(i)
                          .try_as<FrameworkElement>();
            if (fe && fe.Name() == L"PrivacyAnchorBar") {
                gridParent.Children().RemoveAt(i);
                break;
            }
        }
        if (!lease_column::Release(gridParent, g_columnLease)) {
            Wh_Log(L"[Remove] Privacy column lease was not live");
            g_columnLease = {};
        }
    } else {
        g_columnLease = {};
    }

    g_syntheticGrid    = nullptr;
    g_locIcon = nullptr; g_micIcon = nullptr; g_camIcon = nullptr; g_copilotIcon = nullptr;
    g_locSlot = nullptr; g_micSlot = nullptr; g_camSlot = nullptr; g_copilotSlot = nullptr;
    g_locGlowIcon = nullptr; g_micGlowIcon = nullptr; g_camGlowIcon = nullptr; g_copilotGlowIcon = nullptr;
    g_locSlashIcon = nullptr; g_micSlashIcon = nullptr; g_camSlashIcon = nullptr; g_copilotSlashIcon = nullptr;
    g_syntheticParent = nullptr;
    Wh_Log(L"[Remove] PrivacyAnchorBar removed");
}

static void RemoveModUi() {
    g_loadedRevokers->clear();
    ClearPrivacyStates();
    RemoveSyntheticIcons();
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
    // Text() returns a temporary hstring. Own a copy before taking views or
    // indexing; a view of the destroyed temporary is a use-after-free.
    std::wstring text = tb.Text().c_str();

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
            try {
                if (g_unloading) return;
                auto tbRef = sender.try_as<TextBlock>();
                if (!tbRef) return;
                std::wstring newText = tbRef.Text().c_str();
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
            } catch (...) {
                LogCurrentUiException(L"privacy text callback");
            }
        });

    state.visibilityToken = iconView.RegisterPropertyChangedCallback(
        UIElement::VisibilityProperty(),
        [](DependencyObject sender, DependencyProperty) {
            try {
                if (g_unloading) return;
                if (!g_settings.suppressNativeIndicators) return;
                auto iconView = sender.try_as<FrameworkElement>();
                if (!iconView || iconView.Visibility() == Visibility::Collapsed) return;
                iconView.Visibility(Visibility::Collapsed);
                iconView.IsHitTestVisible(false);
            } catch (...) {
                LogCurrentUiException(L"privacy visibility callback");
            }
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
}

// ============================================================
// Apply
// ============================================================

static bool ApplyStyle() {
    Wh_Log(L"[Apply] enter");
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return false; }
    g_taskbarWnd = hWnd;

    XamlRoot xamlRoot = nullptr;
    try { xamlRoot = GetTaskbarXamlRoot(hWnd); } catch (...) { return false; }
    if (!xamlRoot) { Wh_Log(L"[Apply] XamlRoot unavailable"); return false; }

    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return false;

    if (!g_syntheticGrid && !InjectSyntheticIcons(root))
        return false;

    auto sysGrid = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (sysGrid) {
        auto mainStack = FindChildByName(sysGrid, L"MainStack");
        if (mainStack) ScanMainStack(mainStack);
    }
    return true;
}

static bool ApplyOnTaskbarThread() {
    HWND window = FindCurrentProcessTaskbarWnd();
    if (!window) return false;
    g_taskbarWnd = window;
    if (!GetTaskbarXamlRoot(window)) return false;
    if (g_taskbarRestarted.load()) {
        RemoveModUi();
        g_taskbarRestarted.store(false);
    }
    g_loadedRevokers->clear();
    ClearPrivacyStates();
    return ApplyStyle();
}

static void ApplyStyleOnWindowThread() {
    HWND window = g_taskbarWnd ? g_taskbarWnd
                               : FindCurrentProcessTaskbarWnd();
    if (!window) return;
    RunFromWindowThread(
        window, [](void*) { ApplyOnTaskbarThread(); }, nullptr);
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

using TrayUI_StartTaskbar_t = void (WINAPI*)(void*);
static TrayUI_StartTaskbar_t TrayUI_StartTaskbar_Original;

static void WINAPI TrayUI_StartTaskbar_Hook(void* self) {
    TrayUI_StartTaskbar_Original(self);
    if (!g_unloading) {
        g_taskbarWnd = nullptr;
        g_taskbarRestarted.store(true);
        ApplyStyleOnWindowThread();
    }
}

using IconView_IconView_t = void* (WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    void* ret = IconView_IconView_Original(pThis);
    if (g_unloading) return ret;

    try {
        FrameworkElement iconView = nullptr;
        ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                                winrt::put_abi(iconView));
        if (!iconView) return ret;

        g_loadedRevokers->emplace_back();
        auto it = g_loadedRevokers->end(); --it;
        *it = iconView.Loaded(winrt::auto_revoke_t{},
            [it](winrt::Windows::Foundation::IInspectable const& sender, auto const&) {
                try {
                    g_loadedRevokers->erase(it);
                    if (g_unloading) return;
                    auto fe = sender.try_as<FrameworkElement>();
                    if (!fe) return;
                    if (winrt::get_class_name(fe) == L"SystemTray.IconView" &&
                        fe.Name() == L"SystemTrayIcon") {
                        if (g_taskbarRestarted.load()) {
                            ApplyOnTaskbarThread();
                        } else if (!g_syntheticGrid) {
                            auto xamlRoot = fe.XamlRoot();
                            if (xamlRoot) {
                                auto root = xamlRoot.Content().try_as<FrameworkElement>();
                                if (root) InjectSyntheticIcons(root);
                            }
                        }
                        ApplyPrivacyIndicatorBehavior(fe);
                    }
                } catch (...) {
                    LogCurrentUiException(L"IconView Loaded callback");
                }
            });
    } catch (...) {
        LogCurrentUiException(L"IconView constructor hook");
    }

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
    WindhawkUtils::SYMBOL_HOOK taskbarDllHooks[] = {
        { {LR"(const CTaskBand::`vftable'{for `ITaskListWndSite'})"},
          &CTaskBand_ITaskListWndSite_vftable },
        { {LR"(public: virtual class std::shared_ptr<class TaskbarHost> __cdecl CTaskBand::GetTaskbarHost(void)const )"},
          &CTaskBand_GetTaskbarHost_Original },
        { {LR"(public: int __cdecl TaskbarHost::FrameHeight(void)const )"},
          &TaskbarHost_FrameHeight_Original },
        { {LR"(public: void __cdecl std::_Ref_count_base::_Decref(void))"},
          &std__Ref_count_base__Decref_Original },
        { {LR"(public: virtual void __cdecl TrayUI::StartTaskbar(void))"},
          &TrayUI_StartTaskbar_Original, TrayUI_StartTaskbar_Hook },
    };
    return WindhawkUtils::HookSymbols(
        h, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

static bool HookSystemTraySymbols(HMODULE h) {
    // SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll
    WindhawkUtils::SYMBOL_HOOK systemTrayModuleHooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original,
        IconView_IconView_Hook,
        false,
    }};
    return WindhawkUtils::HookSymbols(
        h, systemTrayModuleHooks, ARRAYSIZE(systemTrayModuleHooks));
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
    Wh_Log(L"[Init] Privacy Anchor v1.0");
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
            RemoveModUi();
            // Terminal unload: free the no_destroy optional buffers on the UI
            // thread (RemoveModUi already revoked/cleared their elements).
            g_slotEventStates.reset();
            g_glowAnimationStates.reset();
            g_loadedRevokers.reset();
        }, nullptr);
    } else {
        // No taskbar window means there is no known UI thread on which XAML
        // cleanup is safe. The no_destroy holders intentionally retain their
        // references until process exit instead of releasing them here from
        // Windhawk's unload thread.
        Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] arrangement=%s enabled=%d/%d/%d/%d "
           L"suppressNative=%d cameraHardware=%d glow=%d/%s "
           L"opacity=%d reach=%d speed=%d",
           g_settings.arrangement.c_str(),
           g_settings.location ? 1 : 0, g_settings.microphone ? 1 : 0,
           g_settings.camera ? 1 : 0, g_settings.copilot ? 1 : 0,
           g_settings.suppressNativeIndicators ? 1 : 0,
           g_cameraHardwareDetectionEnabled.load() ? 1 : 0,
           g_settings.glowEnabled ? 1 : 0, g_settings.glowStyle.c_str(),
           g_settings.glowOpacity, g_settings.glowSize,
           g_settings.glowSpeed);

    RequestStateRefresh(RefreshAll | RefreshMonitorSetup);

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void* parameter) {
        HWND window = static_cast<HWND>(parameter);
        if (!GetTaskbarXamlRoot(window)) return;
        RemoveModUi();
        ApplyOnTaskbarThread();
    }, hWnd);
}
