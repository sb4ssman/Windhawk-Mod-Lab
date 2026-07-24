// ==WindhawkMod==
// @id              taskbar-vd-switcher
// @name            Taskbar Virtual Desktop Switcher
// @description     Injects clickable buttons into the taskbar — one per virtual desktop — with configurable grid arrangement for direct switching.
// @version         2.0
// @author          sb4ssman
// @github          https://github.com/sb4ssman
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -lole32 -loleaut32 -lruntimeobject -lversion -luuid
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Virtual Desktop Switcher

A [Windhawk](https://windhawk.net) mod for Windows 11 that injects clickable buttons into the system tray — one per virtual desktop — for instant switching without opening Task View.

![Three desktops with lower master button](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/simple3wlowmaster.png)
*Three desktops with the optional Task View button as a lower sliver.*

![Default tray placement — three numbered buttons, first active](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/simple3.png)
*Default tray placement: three desktops in a row, desktop 1 active.*

![Two desktop compact tray placement](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/simple2.png)
*Compact tray placement with two desktops.*

![Four desktops with master button](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/simple4wmaster.png)
*Four desktops with the optional Task View button.*

![Taller taskbar with right-side grid and lower master button](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/gridonrightwlowermaster.png)
*Taller taskbar: a dense grid with the Task View button as a lower sliver.*

![Left of Start button](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/left-of-start.png)
*Start placement: switcher reserved to the left of Start.*

![Left of Start with Start hidden](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/left-of-start-hidden-start.png)
*Start placement with the Start button hidden.*

![Over Start, nudged above](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/over-above-start.png)
*Overlay mode can be nudged up with the vertical offset setting.*

![Over Start, nudged below](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/over-below-start.png)
*Overlay mode can also be nudged down.*

![Right of Start with Start hidden](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/taskbar-vd-switcher/assets/right-of-start-hidden-start.png)
*Right-of-Start placement when the Start button is hidden.*

## Features

- Numbered, roman-numeral, indicator-symbol, or custom-label buttons
- Automatic grid that fits the buttons to the taskbar's height, or an arrangement you write yourself
- Optional Task View button as a full column, a sliver, or one more button in the grid
- Highlights the active desktop immediately on switch
- Buttons appear and disappear as desktops are added or removed
- Five tray positions, plus experimental Start-adjacent and Start-overlay positions
- Configurable size, spacing, colors, opacity, and shine effect
- Per-state text color, font size and family, corner radius, bold, and border
- Native checked states that can be targeted by Windows 11 Taskbar Styler
- Tooltip on each button shows the desktop's display name
- Option to hide the bar entirely when only one desktop exists
- Experimental option to also show the switcher on secondary monitors' taskbars

## Upgrading from 1.x

Version 2.0 reorganizes every setting into groups — Placement, Content, Layout,
Size, Adjust, Surface, State, Behavior — so this mod matches the rest of the
family. Windhawk cannot carry values across renamed keys, so **your previous
customizations are not migrated; re-apply them once after updating.**

The layout settings collapsed into a single **Arrangement** field. Grid mode,
smart layout, rows, columns, primary axis, cross alignment, the four padding
sides, the vertical offset, and both nudge strings are gone; see below for what
replaced them.

## The Arrangement field

`Layout` → `Arrangement` decides how the buttons are placed, and it is the only
field that does. Its default value is the word `auto`:

- **`auto`** fits the buttons to the available taskbar height. `Fill order`
  chooses whether they fill across rows or down columns; `Short row or column`
  aligns a ragged last group. The shape itself is worked out for you: the mod
  takes the narrowest grid that fits the height, preferring the one that wastes
  the fewest slots — four desktops on a double-height taskbar become a 2×2
  block, not a lopsided 3+1.
- **Anything else** is an arrangement you write. Names sit side by side with
  `|` and stack with `,`, and parentheses group them:

  ```text
  1, 2 | 3, 4        a 2x2 block
  1 | 2 | 3 | 4      a single row
  1, 2, 3, 4         a single column
  master | (1, 2)    Task View button left of a stacked pair
  ```

  Buttons are named by desktop number; the Task View button is `master` or
  `taskview`. `desktop2` also works as a readable alias for `2`, and names are
  case-insensitive. A separator is always required — `1 (2 | 3)` is an error,
  not a shorthand for `1 | (2 | 3)`.

Every time the layout is applied, the arrangement `auto` produced is written to
the Windhawk log, along with which desktop each number refers to
(`tokens: 1=Home  2=Work`). Copy the arrangement line into the Arrangement
field and you have the automatic layout as a starting point to edit — the
automatic and manual paths are the same field and the same syntax. If what you
write doesn't parse, the log says what was expected and where, and the
automatic arrangement is used until you fix it.

**Nudging.** Append a pixel offset to any name to move just that button:

```text
1[+2,-1] | 2 | 3       desktop 1 moves 2px right and 1px up
(1, 2) | master[0,2]   Task View button drops 2px
```

A parenthesized group takes an offset too, moving everything inside it:

```text
(1, 2)[3,0] | 3        the stacked pair moves 3px right, 3 stays put
```

Offsets are cosmetic. Nothing else shifts, and the group's overall size does
not change. To move the whole group instead, use `Adjust` → horizontal and
vertical offset.

**Desktops you create later.** An arrangement you write names the desktops that
existed when you wrote it. Create another one and it is in no group, so by
default it is appended after your arrangement rather than vanishing — the log
says when that happened, so you can fold it in when you next edit. Set
`Layout` → `Newly created desktops` to *Leave them out* if you would rather
your arrangement be the whole truth. `auto` always includes every desktop.

## The Task View button

`Content` → `Task View button placement` decides where it goes: a column
**before** or **after** the desktop buttons, a row **above** or **below** them,
or the **last button in the grid**. This applies whether the layout came from
`auto` or from an arrangement you wrote — write `master` in your arrangement
and you place it exactly, and the setting steps aside.

For the column and row placements, `Size` → `Task View button thickness` is how
thick it is: its **width** as a column, its **height** as a row. `Task View
button length` is how far it runs along the desktop buttons, and `0` — the
default — means match them exactly, so it is a full-height column or a
full-width sliver however many desktops you have. Give the length a value to
make it shorter; it is then centered by `Short row or column`.

`Task View button gap` puts extra distance between it and the desktop buttons,
on top of the normal spacing — positive pushes it further away whichever side
it is on, negative pulls it closer or over them. It moves the button **without
resizing the group**, which is the useful part: push a sliver below far enough
and it hangs past the bottom of the taskbar so only its leading edge shows,
rather than the whole group growing and re-centering. On a column, a few pixels
of gap simply sets it apart from the set.

**Last button in the grid** ignores thickness, length, and gap, and sizes it
like a desktop button so it flows with them as one more cell — `1, 4 | 2, 5 |
3, ⊞`. Use it when you want the Task View button to read as part of the set
rather than as a bar alongside it; it keeps its own label and font.

All of this applies when the arrangement does not name the button. Write
`master` yourself and you are placing it — add your own offset there if you
want the gap, like `(1 | 2 | 3), master[0,8]`.

## Settings

### Placement

| Setting | Default | Description |
|---------|---------|-------------|
| Position | After clock | Tray position, or left of / over / right of Start |
| Show on all taskbars | Off | Experimental; also injects into secondary monitors' taskbars |

### Content

| Setting | Default | Description |
|---------|---------|-------------|
| Label format | Numbers | Numbers · Roman numerals · Indicator symbols · Custom labels |
| Custom labels | *(empty)* | Comma-separated, e.g. `H,W,M` |
| Active indicator symbol | ● | Current desktop's symbol in Indicator symbols mode |
| Inactive indicator symbol | ○ | Other desktops' symbol; paste 🟢 above and 🔴 here for a stoplight |
| Task View button | Off | Adds a button that opens Task View for previewing, creating, or closing desktops |
| Task View button label | ⊞ | Text shown on that button |
| Task View button placement | After | Column before/after, row above/below, or last button in the grid |

### Layout

| Setting | Default | Description |
|---------|---------|-------------|
| Arrangement | `auto` | `auto`, or an arrangement you write — see above |
| Fill order | Fill rows first | Used by `auto` |
| Short row or column | Center | Used by `auto`; start, center, or end |
| Newly created desktops | Add them after | Or leave them out; only applies to a written arrangement |

### Size

| Setting | Default | Description |
|---------|---------|-------------|
| Button width | 20 px | |
| Button height | 22 px | |
| Button spacing | 2 px | Gap between buttons along each axis |
| Task View button thickness | 14 px | Width as a column, height as a sliver; unused in the grid placement |
| Task View button length | 0 px | 0 matches the desktop buttons exactly |
| Task View button gap | 0 px | Extra distance from the desktop buttons; moves it without resizing the group |

### Adjust

| Setting | Default | Description |
|---------|---------|-------------|
| Horizontal padding | 0 px | Reserved on both sides of the group |
| Vertical padding | 0 px | Reserved above and below the group |
| Horizontal offset | 0 px | Moves the group; reserves no space |
| Vertical offset | 0 px | Moves the group up (negative) or down (positive) |

### Surface

| Setting | Default | Description |
|---------|---------|-------------|
| Font size | 10 pt | |
| Font family | *(native)* | For desktop labels and indicator symbols |
| Hover background color | *(automatic)* | Empty brightens each button's own background |
| Click background color | *(automatic)* | Empty darkens each button's own background |
| Border color | *(native)* | |
| Border thickness | 0 px | |
| Corner radius | 4 px | 0 = square, 4 = Windows default |
| Opacity | 100 | Lower values let the taskbar show through |
| Shine effect | Off | Gradient highlight on buttons with custom colors |
| Task View font family | *(native)* | Independent font for the Task View label |

### State

| Setting | Default | Description |
|---------|---------|-------------|
| Active desktop text color | *(native)* | |
| Inactive button text color | *(native)* | |
| Active desktop color | `accent` | Empty keeps the plain native surface |
| Inactive button color | *(native)* | |
| Bold the active desktop label | Off | |

### Behavior

| Setting | Default | Description |
|---------|---------|-------------|
| Hide when only one desktop | Off | |

All color settings accept `#RRGGBB` or `#AARRGGBB` hex (the alpha byte is
honored), the generics `accent`, `accentLight`, and `accentDark` for the
Windows accent shades, or `transparent` for a fully transparent surface —
nothing drawn, element still present and clickable. Leaving a color empty
keeps the native behavior described for that setting — including the Active
desktop color, where empty means the current desktop's button keeps the plain
native surface with no highlight at all.

## Taskbar Styler

Desktop buttons are XAML `ToggleButton` controls named `VdBtn_0`, `VdBtn_1`,
and so on. The current desktop has `IsChecked=true`, exposing the native
`Checked`, `CheckedPointerOver`, and `CheckedPressed` states. Taskbar Styler
can target every indicator's template presenter with:

```text
Grid#VdSwitcherBar > ToggleButton > ContentPresenter#ContentPresenter@CommonStates
```

State-qualified styles such as `Background@Checked`,
`Background@CheckedPointerOver`, and `Background@CheckedPressed` then apply
without inferring the active desktop from its color.

## Known limitations

- Multi-monitor support is experimental and off by default: secondary taskbars use the tray positions only (Start positions stay on the primary taskbar), and they are discovered as their tray icons load — after enabling the option, an Explorer restart (or toggling the mod off and on) may be needed before the buttons appear on other monitors
- Buttons may not appear until the mod injects on the first tray icon load; retry loop runs up to 5 times at 2-second intervals

## Credits and inspirations

This mod builds directly on patterns established by several community mods:

**[taskbar-empty-space-clicks](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-empty-space-clicks.wh.cpp)** — source of the `SwitchVirtualDesktop()` COM vtable pattern, build-specific IIDs for `IVirtualDesktopManagerInternal`, and the `IObjectArray` desktop enumeration approach.

**[taskbar-desktop-indicator](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-desktop-indicator.wh.cpp)** — reference for reading the current virtual desktop from the registry (session-scoped `VirtualDesktopIDs` + `CurrentVirtualDesktop` keys) and the notification cookie / `IVirtualDesktopNotificationService` registration pattern.

**[Vertical OmniButton archive](https://github.com/sb4ssman/Windhawk-Mod-Lab/blob/main/omnibutton-customizer/archive/vertical-omnibutton-v1.4.wh.cpp)** (this lab, by sb4ssman) — source of the `GetTaskbarXamlRoot` boilerplate, `RunFromWindowThread` dispatcher, `FindCurrentProcessTaskbarWnd`, and the `IconView::IconView` hook-and-retry injection pattern.

**[windows-11-taskbar-styler](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/windows-11-taskbar-styler.wh.cpp)** — reference for the `SystemTrayFrameGrid` XAML tree structure and element names (`ShowDesktopStack`, `NotificationCenterButton`, `ControlCenterButton`, `NotifyIconStack`).

**[Windhawk](https://windhawk.net)** by [m417z](https://github.com/m417z) — the modding platform that makes all of this possible.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- Placement:
  - Position: "afterClock"
    $name: Position
    $description: Where to place the switcher on the taskbar.
    $options:
    - "beforeIcons": "Before notification icons"
    - "beforeOmni": "Before network, volume, and battery"
    - "beforeClock": "Before clock"
    - "afterClock": "After clock"
    - "afterShowDesktop": "After Show Desktop"
    - "leftOfStart": "Left of Start (experimental)"
    - "overStart": "Over Start (experimental)"
    - "rightOfStart": "Right of Start (experimental)"
  - AllTaskbars: false
    $name: Show on all taskbars
    $description: >-
      Experimental. Also injects the switcher into secondary monitors'
      taskbars, in the same position. Tray positions only - the Start
      positions stay on the primary taskbar. Secondary taskbars are
      discovered as their tray icons load, so after enabling this you may
      need to restart Explorer before the buttons appear on other monitors.
  $name: Placement

- Content:
  - LabelFormat: "number"
    $name: Label format
    $options:
    - "number": "Numbers  1  2  3"
    - "roman": "Roman numerals  I  II  III"
    - "symbol": "Indicator symbols  ●  ○  ○"
    - "custom": "Custom labels"
  - CustomLabels: ""
    $name: Custom labels
    $description: >-
      Comma-separated, e.g. "H,W,M". Used when Label format is Custom. Falls
      back to numbers if labels run out.
  - ActiveSymbol: "●"
    $name: Active indicator symbol
    $description: >-
      Shown for the current desktop when Label format is Indicator symbols.
      For a stoplight look, paste 🟢 here and 🔴 below.
  - InactiveSymbol: "○"
    $name: Inactive indicator symbol
    $description: >-
      Shown for the other desktops when Label format is Indicator symbols.
      For a stoplight look, paste 🔴 here and 🟢 above.
  - TaskViewButton: false
    $name: Task View button
    $description: >-
      Adds a button that opens Task View (Win+Tab), where you can preview all
      desktops and create or close them.
  - TaskViewLabel: "⊞"
    $name: Task View button label
  - TaskViewPlacement: "after"
    $name: Task View button placement
    $description: >-
      Where the Task View button goes when your arrangement does not name it.
      Write "master" in the arrangement yourself to place it exactly, and this
      is ignored.
    $options:
    - "before": "Column before the desktop buttons"
    - "after": "Column after the desktop buttons"
    - "above": "Row above the desktop buttons"
    - "below": "Row below the desktop buttons"
    - "inGrid": "Last button in the grid, same size as the others"
  $name: Content

- Layout:
  - Arrangement: "auto"
    $name: Arrangement
    $description: >-
      "auto" fits the buttons to the available taskbar height. Anything else
      is an explicit arrangement: names side by side with "|", stacked with
      ",", grouped with parentheses - "1, 2 | 3, 4" is a 2x2 block. Buttons
      are named by desktop number ("desktop2" also works), plus "master" or
      "taskview" for the Task View button. Append a pixel offset to nudge one button,
      "1[+2,-1]", or a whole group, "(1, 2)[3,0]". Every time the layout is
      applied, the arrangement "auto" produced is written to the Windhawk log
      along with which desktop each number is, so you can paste it here and
      edit it. If what you type does not parse, the log says what was expected
      and where, and "auto" is used until you fix it.
  - FillOrder: "rows"
    $name: Fill order
    $description: Used by "auto". Whether buttons fill across rows or down columns first.
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
    $name: Newly created desktops
    $description: >-
      What happens when you create a desktop that your own arrangement does not
      name. Only applies when you have written an arrangement - "auto" always
      includes every desktop.
    $options:
    - "append": "Add them after the arrangement"
    - "ignore": "Leave them out until I add them"
  $name: Layout

- Size:
  - ItemWidth: 20
    $name: Button width (px)
  - ItemHeight: 22
    $name: Button height (px)
  - ItemSpacing: 2
    $name: Button spacing (px)
    $description: Gap between buttons along each axis.
  - TaskViewSize: 14
    $name: Task View button thickness (px)
    $description: >-
      How thick the Task View button is: its width when it sits beside the
      desktop buttons, its height when it sits above or below them. Not used
      when its placement is "Last button in the grid" - it is then sized like
      a desktop button.
  - TaskViewSpan: 0
    $name: Task View button length (px)
    $description: >-
      How far it runs along the desktop buttons. 0 matches them exactly - a
      full-height column beside them, or a full-width sliver above or below.
      Any other value is a fixed length, centered by Short row or column.
  - TaskViewGap: 0
    $name: Task View button gap (px)
    $description: >-
      Extra distance between the Task View button and the desktop buttons, on
      top of the normal button spacing. Positive pushes it further away,
      negative pulls it closer or over them. It moves without resizing the
      group, so a sliver can hang past the edge of the taskbar and show only
      its leading edge. Not used when its placement is "Last button in the
      grid", or when you name it in your own arrangement - write your own
      offset there, like "master[0,3]".
  $name: Size

- Adjust:
  - PadX: 0
    $name: Horizontal padding (px)
    $description: Space reserved on both sides of the button group.
  - PadY: 0
    $name: Vertical padding (px)
    $description: Space reserved above and below the button group.
  - OffsetX: 0
    $name: Horizontal offset (px)
    $description: Moves the whole group. Does not reserve space.
  - OffsetY: 0
    $name: Vertical offset (px)
    $description: Moves the whole group up (negative) or down (positive) from centered.
  $name: Adjust

- Surface:
  - FontSize: 10
    $name: Font size (pt)
  - FontFamily: ""
    $name: Font family
    $description: >-
      Font for the desktop labels and indicator symbols. Empty uses the
      native font. For example, Segoe UI Emoji.
  - HoverBackgroundColor: ""
    $name: Hover background color
    $description: >-
      Hex (#RRGGBB or #AARRGGBB), accent / accentLight / accentDark, or
      transparent, to force one shared hover color. Empty brightens each
      button's own background; buttons on the native surface keep the native
      hover.
  - PressedBackgroundColor: ""
    $name: Click background color
    $description: >-
      Hex, accent / accentLight / accentDark, or transparent, to force one
      shared pressed color. Empty darkens each button's own background;
      buttons on the native surface keep the native pressed state.
  - BorderColor: ""
    $name: Border color
    $description: Hex, accent / accentLight / accentDark, or transparent. Empty uses the native border.
  - BorderThickness: 0
    $name: Border thickness (px)
  - CornerRadius: 4
    $name: Corner radius (px)
    $description: 0 is square; 4 is the Windows default.
  - Opacity: 100
    $name: Opacity (%)
    $description: 100 is fully opaque; lower values let the taskbar show through.
  - ShineEffect: false
    $name: Shine effect
    $description: Adds a gradient highlight to buttons with a custom color.
  - TaskViewFontFamily: ""
    $name: Task View font family
    $description: Font for the Task View button label. Empty uses the native font.
  $name: Surface

- State:
  - ActiveTextColor: ""
    $name: Active desktop text color
    $description: Hex, accent / accentLight / accentDark, or transparent. Empty uses the native text color.
  - InactiveTextColor: ""
    $name: Inactive button text color
    $description: Hex, accent / accentLight / accentDark, or transparent. Empty uses the native text color.
  - ActiveBackgroundColor: "accent"
    $name: Active desktop color
    $description: >-
      Background for the current desktop's button. Hex, accent / accentLight
      / accentDark, or transparent. Empty keeps the native button surface,
      matching the other buttons.
  - InactiveBackgroundColor: ""
    $name: Inactive button color
    $description: >-
      Background for the other desktops' buttons. Hex, accent / accentLight /
      accentDark, or transparent. Empty keeps the native button surface.
  - ActiveBold: false
    $name: Bold the active desktop label
  $name: State

- Behavior:
  - HideWhenSingle: false
    $name: Hide when only one desktop
    $description: Don't show the buttons when there is only one virtual desktop.
  $name: Behavior
*/
// ==/WindhawkModSettings==

#undef GetCurrentTime

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Automation.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Media.h>

#include <atomic>
#include <list>
#include <optional>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <thread>
#include <functional>
#include <algorithm>
#include <climits>
#include <cmath>
#include <cwchar>
#include <cstdlib>
#include <exception>

#include <windhawk_utils.h>
#include <combaseapi.h>
#include <winver.h>

using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Automation;
using namespace winrt::Windows::UI::Xaml::Controls;
using namespace winrt::Windows::UI::Xaml::Media;
using winrt::Windows::UI::Xaml::Controls::Primitives::ButtonBase;
using winrt::Windows::UI::Xaml::Controls::Primitives::ToggleButton;

// ============================================================
// Settings
// ============================================================

// Field names mirror the settings contract in _templates/settings-profiles.md:
// one nested group per concern, canonical keys in canonical order.
struct ModSettings {
    // Placement
    std::wstring position          = L"afterClock";
    bool         allTaskbars       = false;
    // Content
    std::wstring labelFormat       = L"number";
    std::wstring customLabels      = L"";
    std::wstring activeSymbol      = L"●";
    std::wstring inactiveSymbol    = L"○";
    bool         taskViewButton    = false;
    std::wstring taskViewLabel     = L"⊞";
    std::wstring taskViewPlacement = L"after";
    // Layout
    std::wstring arrangement       = L"auto";
    std::wstring fillOrder         = L"rows";
    std::wstring justify           = L"center";
    std::wstring newItems          = L"append";
    // Size
    int          itemWidth         = 20;
    int          itemHeight        = 22;
    int          itemSpacing       = 2;
    int          taskViewSize      = 14;
    int          taskViewSpan      = 0;
    int          taskViewGap       = 0;
    // Adjust
    int          padX              = 0;
    int          padY              = 0;
    int          offsetX           = 0;
    int          offsetY           = 0;
    // Surface
    int          fontSize          = 10;
    std::wstring fontFamily        = L"";
    std::wstring hoverBackgroundColor;
    std::wstring pressedBackgroundColor;
    std::wstring borderColor       = L"";
    int          borderThickness   = 0;
    int          cornerRadius      = 4;
    int          opacity           = 100;
    bool         shineEffect       = false;
    std::wstring taskViewFontFamily = L"";
    // State
    std::wstring activeTextColor   = L"";
    std::wstring inactiveTextColor = L"";
    std::wstring activeBackgroundColor   = L"accent";
    std::wstring inactiveBackgroundColor = L"";
    bool         activeBold        = false;
    // Behavior
    bool         hideWhenSingle    = false;
};
// ModSettings holds only std::wstring/int/bool, so its destructor is safe to
// run at process shutdown; no [[clang::no_destroy]] needed, and adding it would
// leak the string buffers on every normal unload.
ModSettings g_settings;  // exit-time-safe: heap-only

static void LoadSettings() {
    auto Str = [](const wchar_t* k) {
        PCWSTR p = Wh_GetStringSetting(k);
        std::wstring r = p;
        Wh_FreeStringSetting(p);
        return r;
    };
    auto Int = [](const wchar_t* k) { return Wh_GetIntSetting(k); };
    auto Bool = [](const wchar_t* k) { return Wh_GetIntSetting(k) != 0; };

    g_settings.position          = Str(L"Placement.Position");
    g_settings.allTaskbars       = Bool(L"Placement.AllTaskbars");

    g_settings.labelFormat       = Str(L"Content.LabelFormat");
    g_settings.customLabels      = Str(L"Content.CustomLabels");
    g_settings.activeSymbol      = Str(L"Content.ActiveSymbol");
    g_settings.inactiveSymbol    = Str(L"Content.InactiveSymbol");
    g_settings.taskViewButton    = Bool(L"Content.TaskViewButton");
    g_settings.taskViewLabel     = Str(L"Content.TaskViewLabel");
    g_settings.taskViewPlacement = Str(L"Content.TaskViewPlacement");

    g_settings.arrangement       = Str(L"Layout.Arrangement");
    g_settings.fillOrder         = Str(L"Layout.FillOrder");
    g_settings.justify           = Str(L"Layout.Justify");
    g_settings.newItems          = Str(L"Layout.NewItems");

    g_settings.itemWidth         = std::max(1, Int(L"Size.ItemWidth"));
    g_settings.itemHeight        = std::max(1, Int(L"Size.ItemHeight"));
    g_settings.itemSpacing       = std::max(0, Int(L"Size.ItemSpacing"));
    g_settings.taskViewSize      = std::max(1, Int(L"Size.TaskViewSize"));
    g_settings.taskViewSpan      = std::max(0, Int(L"Size.TaskViewSpan"));
    g_settings.taskViewGap       = Int(L"Size.TaskViewGap");

    g_settings.padX              = std::max(0, Int(L"Adjust.PadX"));
    g_settings.padY              = std::max(0, Int(L"Adjust.PadY"));
    g_settings.offsetX           = Int(L"Adjust.OffsetX");
    g_settings.offsetY           = Int(L"Adjust.OffsetY");

    g_settings.fontSize          = Int(L"Surface.FontSize");
    g_settings.fontFamily        = Str(L"Surface.FontFamily");
    g_settings.hoverBackgroundColor   = Str(L"Surface.HoverBackgroundColor");
    g_settings.pressedBackgroundColor = Str(L"Surface.PressedBackgroundColor");
    g_settings.borderColor       = Str(L"Surface.BorderColor");
    g_settings.borderThickness   = Int(L"Surface.BorderThickness");
    g_settings.cornerRadius      = Int(L"Surface.CornerRadius");
    g_settings.opacity           = Int(L"Surface.Opacity");
    g_settings.shineEffect       = Bool(L"Surface.ShineEffect");
    g_settings.taskViewFontFamily = Str(L"Surface.TaskViewFontFamily");

    g_settings.activeTextColor   = Str(L"State.ActiveTextColor");
    g_settings.inactiveTextColor = Str(L"State.InactiveTextColor");
    g_settings.activeBackgroundColor   = Str(L"State.ActiveBackgroundColor");
    g_settings.inactiveBackgroundColor = Str(L"State.InactiveBackgroundColor");
    g_settings.activeBold        = Bool(L"State.ActiveBold");

    g_settings.hideWhenSingle    = Bool(L"Behavior.HideWhenSingle");

    auto shownColor = [](std::wstring const& value) {
        return value.empty() ? L"<empty/automatic>" : value.c_str();
    };
    Wh_Log(L"[Settings] colors active=%ls inactive=%ls hover=%ls pressed=%ls border=%ls",
           shownColor(g_settings.activeBackgroundColor),
           shownColor(g_settings.inactiveBackgroundColor),
           shownColor(g_settings.hoverBackgroundColor),
           shownColor(g_settings.pressedBackgroundColor),
           shownColor(g_settings.borderColor));
}
// ============================================================
// Globals
// ============================================================

static std::atomic<bool> g_unloading{false};
static HWND              g_taskbarWnd      = nullptr;
[[clang::no_destroy]] static Grid g_buttonGrid = nullptr;
[[clang::no_destroy]] static FrameworkElement g_injectionParent = nullptr;
static int               g_injectedColumn  = -1;
static bool              g_startOverlayMode = false;
[[clang::no_destroy]] static FrameworkElement g_startOverlayRoot = nullptr;
[[clang::no_destroy]] static FrameworkElement g_startOverlayStart = nullptr;
static winrt::event_token g_startOverlayLayoutToken{};
[[clang::no_destroy]] static FrameworkElement g_taskItemsPanel = nullptr;
static Thickness         g_taskItemsPanelOriginalMargin{};
static double            g_startButtonOriginalX = -1.0;
static std::atomic<int>  g_currentDesktop{0};
static std::atomic<int>  g_desktopCount{1};

static HANDLE g_notificationThread    = nullptr;
static HANDLE g_notificationStopEvent = nullptr;
static DWORD  g_notificationCookie    = 0;

static HANDLE g_retryThread    = nullptr;
static HANDLE g_retryStopEvent = nullptr;

static std::atomic<bool> g_systemTrayModuleHooked{false};
static std::atomic<int>  g_activeSwitchThreads{0};
[[clang::no_destroy]] static std::optional<std::list<FrameworkElement::Loaded_revoker>>
    g_autoRevokerList{std::in_place};

struct ButtonEventState {
    Grid owner{nullptr};
    ButtonBase button{nullptr};
    winrt::event_token clickToken{};
};
[[clang::no_destroy]] static std::optional<std::vector<ButtonEventState>>
    g_buttonEventStates{std::in_place};

// Forward declarations
static void ApplyAllSettings();
static void ApplyAllSettingsOnWindowThread();
static void RebuildButtonGrid();
static void RemoveButtonGrid();
static void StopNotificationThread();
static void StopRetryThread();
static void HandleLoadedModuleIfSystemTray(HMODULE hModule, LPCWSTR lpLibFileName);

// ============================================================
// Explorer / twinui build detection
// ============================================================

static WORD g_explorerBuild    = 0;
static WORD g_explorerRevision = 0;
static WORD g_twinuiBuild      = 0;

static void DetectExplorerBuild() {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    DWORD dummy;
    DWORD sz = GetFileVersionInfoSizeW(path, &dummy);
    if (!sz) return;
    std::vector<BYTE> buf(sz);
    if (!GetFileVersionInfoW(path, 0, sz, buf.data())) return;
    VS_FIXEDFILEINFO* fi = nullptr; UINT fs = 0;
    if (!VerQueryValueW(buf.data(), L"\\", (void**)&fi, &fs)) return;
    g_explorerBuild    = HIWORD(fi->dwFileVersionLS);
    g_explorerRevision = LOWORD(fi->dwFileVersionLS);
    Wh_Log(L"[Init] Explorer build %u rev %u", g_explorerBuild, g_explorerRevision);
}

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

static bool LoadTwinuiBuild() {
    if (g_twinuiBuild) return true;
    HMODULE h = GetModuleHandleW(L"twinui.pcshell.dll");
    if (!h) return false;
    VS_FIXEDFILEINFO* fi = GetModuleVersionInfo(h, nullptr);
    if (!fi) return false;
    g_twinuiBuild = HIWORD(fi->dwFileVersionLS);
    Wh_Log(L"[VD] twinui.pcshell.dll build %u", g_twinuiBuild);
    return true;
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

// ============================================================
// GetTaskbarXamlRoot boilerplate (from vertical-omnibutton)
// ============================================================

using RunFromWindowThreadProc_t = void (*)(void*);

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

static bool InvokeWindowThreadProc(RunFromWindowThreadProc_t proc,
                                   void* procParam) {
    try {
        proc(procParam);
        return true;
    } catch (...) {
        LogCurrentUiException(L"UI callback");
        return false;
    }
}

static bool RunFromWindowThread(HWND hWnd, RunFromWindowThreadProc_t proc, void* procParam) {
    static const UINT kMsg = RegisterWindowMessage(L"Windhawk_RunFromWindowThread_" WH_MOD_ID);
    struct Param {
        RunFromWindowThreadProc_t proc;
        void* procParam;
        bool succeeded = false;
    };
    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (!dwThreadId) return false;
    if (dwThreadId == GetCurrentThreadId())
        return InvokeWindowThreadProc(proc, procParam);
    HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (nCode == HC_ACTION) {
            const CWPSTRUCT* cwp = (const CWPSTRUCT*)lParam;
            if (cwp->message == RegisterWindowMessageW(L"Windhawk_RunFromWindowThread_" WH_MOD_ID)) {
                auto* p = (Param*)cwp->lParam;
                p->succeeded = InvokeWindowThreadProc(p->proc, p->procParam);
            }
        }
        return CallNextHookEx(nullptr, nCode, wParam, lParam);
    }, nullptr, dwThreadId);
    if (!hook) return false;
    Param param{ proc, procParam };
    SendMessage(hWnd, kMsg, 0, (LPARAM)&param);
    UnhookWindowsHookEx(hook);
    return param.succeeded;
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

using CTaskBand_GetTaskbarHost_t = void* (WINAPI*)(void* pThis, void* taskbarHostSharedPtr);
CTaskBand_GetTaskbarHost_t CTaskBand_GetTaskbarHost_Original;

using TaskbarHost_FrameHeight_t = int (WINAPI*)(void* pThis);
TaskbarHost_FrameHeight_t TaskbarHost_FrameHeight_Original;

using std__Ref_count_base__Decref_t = void (WINAPI*)(void* pThis);
std__Ref_count_base__Decref_t std__Ref_count_base__Decref_Original;

static void* CTaskBand_ITaskListWndSite_vftable = nullptr;

static XamlRoot GetTaskbarXamlRoot(HWND hTaskbarWnd) {
    // Guard: symbols must be resolved before any dereference.
    if (!CTaskBand_GetTaskbarHost_Original || !TaskbarHost_FrameHeight_Original ||
        !std__Ref_count_base__Decref_Original ||
        !CTaskBand_ITaskListWndSite_vftable)
        return nullptr;

    HWND hTaskSwWnd = (HWND)GetProp(hTaskbarWnd, L"TaskbandHWND");
    if (!hTaskSwWnd) return nullptr;
    void* taskBand = (void*)GetWindowLongPtr(hTaskSwWnd, 0);
    // Guard: taskBand is null during early startup before the taskband stores its this-pointer.
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
    // Guard: iunk is null during early startup before the TaskbarElement is set at offset.
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
// XAML helpers
// ============================================================

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

// ============================================================
// VD COM notification infrastructure
// ============================================================

const CLSID CLSID_ImmersiveShell = {
    0xc2f03a33,0x21f5,0x47fa,{0xb4,0xbb,0x15,0x63,0x62,0xa2,0xf2,0x39}
};
const GUID SID_VirtualDesktopNotificationService = {
    0xa501fdec,0x4a09,0x464c,{0xae,0x4e,0x1b,0x9c,0x21,0xb8,0x49,0x18}
};
const GUID IID_IVirtualDesktopNotificationService_G = {
    0x0cd45e71,0xd927,0x4f15,{0x8b,0x0a,0x8f,0xef,0x52,0x53,0x37,0xbf}
};

MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
IVirtualDesktopNotificationService_I : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE Register(IUnknown*, DWORD*) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unregister(DWORD) = 0;
};

struct NotifConfig {
    int64_t iidPart1 = 0, iidPart2 = 0;
    int methodCount = 0, createdIdx = -1, destroyedIdx = -1, currentChangedIdx = -1;
    bool hasMonitors = false;
};

struct NotifObject {
    void** vtable  = nullptr;
    LONG refCount  = 1;
};

static NotifConfig GetNotifConfig() {
    if (g_explorerBuild < 22000) return {};
    if (g_explorerBuild < 22483 || (g_explorerBuild == 22621 && g_explorerRevision < 2215))
        return { 5481970284372180562ll, -1679294552252794956ll, 13, 7, 9, 11, true };
    if (g_explorerBuild < 22631 || (g_explorerBuild == 22631 && g_explorerRevision < 3085))
        return { 5123538856297626140ll,  8491238173783613346ll, 14, 6, 8, 10, false };
    return     { 5308375338100058445ll, -2401892766147978065ll, 14, 6, 8, 10, false };
}

static bool IsOurNotifIface(REFIID riid) {
    auto cfg = GetNotifConfig();
    if (!cfg.methodCount) return false;
    auto p = reinterpret_cast<const int64_t*>(&riid);
    return p[0] == cfg.iidPart1 && p[1] == cfg.iidPart2;
}

static HRESULT STDMETHODCALLTYPE Notif_QI(NotifObject* p, REFIID riid, void** ppv) {
    if (!ppv) return E_POINTER; *ppv = nullptr;
    static const GUID IID_IUnknown_ = {0,0,0,{0xc0,0,0,0,0,0,0,0x46}};
    if (InlineIsEqualGUID(riid, IID_IUnknown_) || IsOurNotifIface(riid)) {
        *ppv = p; InterlockedIncrement(&p->refCount); return S_OK;
    }
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE Notif_AddRef(NotifObject* p) {
    return (ULONG)InterlockedIncrement(&p->refCount);
}
static ULONG STDMETHODCALLTYPE Notif_Release(NotifObject* p) {
    LONG r = InterlockedDecrement(&p->refCount);
    if (r == 0) { delete[] p->vtable; delete p; }
    return (ULONG)std::max(r, 0L);
}
static HRESULT STDMETHODCALLTYPE Notif_HandleUpdate() {
    if (g_unloading || !g_taskbarWnd) return S_OK;
    RunFromWindowThread(g_taskbarWnd, [](void*) {
        if (!g_unloading) RebuildButtonGrid();
    }, nullptr);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE Notif_NoOp() { return S_OK; }
static HRESULT STDMETHODCALLTYPE Notif_CountChanged(NotifObject*) { return Notif_HandleUpdate(); }
static HRESULT STDMETHODCALLTYPE Notif_CurrentChanged(NotifObject*) { return Notif_HandleUpdate(); }
static HRESULT STDMETHODCALLTYPE Notif_CurrentChangedWithMonitors(NotifObject*, void*, void*, void*) {
    return Notif_HandleUpdate();
}

static NotifObject* CreateNotifObject() {
    auto cfg = GetNotifConfig();
    if (cfg.methodCount == 0 || cfg.currentChangedIdx < 0) return nullptr;
    auto* obj = new (std::nothrow) NotifObject();
    if (!obj) return nullptr;
    obj->vtable = new (std::nothrow) void*[cfg.methodCount];
    if (!obj->vtable) { delete obj; return nullptr; }
    for (int i = 0; i < cfg.methodCount; i++) obj->vtable[i] = (void*)&Notif_NoOp;
    obj->vtable[0] = (void*)&Notif_QI;
    obj->vtable[1] = (void*)&Notif_AddRef;
    obj->vtable[2] = (void*)&Notif_Release;
    if (cfg.createdIdx >= 0)   obj->vtable[cfg.createdIdx]   = (void*)&Notif_CountChanged;
    if (cfg.destroyedIdx >= 0) obj->vtable[cfg.destroyedIdx] = (void*)&Notif_CountChanged;
    obj->vtable[cfg.currentChangedIdx] = cfg.hasMonitors
        ? (void*)&Notif_CurrentChangedWithMonitors
        : (void*)&Notif_CurrentChanged;
    return obj;
}

static NotifObject* g_notifObject = nullptr;

static DWORD WINAPI NotificationThreadProc(void*) {
    auto cfg = GetNotifConfig();
    if (cfg.methodCount == 0) {
        Wh_Log(L"[Notif] Unsupported build (explorer %u)", g_explorerBuild);
        return 0;
    }
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return 0;

    IServiceProvider* svc = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER,
                                IID_IServiceProvider, (void**)&svc)) || !svc) {
        CoUninitialize(); return 0;
    }
    IVirtualDesktopNotificationService_I* notifSvc = nullptr;
    svc->QueryService(SID_VirtualDesktopNotificationService,
                      IID_IVirtualDesktopNotificationService_G, (void**)&notifSvc);
    svc->Release();
    if (!notifSvc) { CoUninitialize(); return 0; }

    g_notifObject = CreateNotifObject();
    if (!g_notifObject) { notifSvc->Release(); CoUninitialize(); return 0; }

    HRESULT hr = notifSvc->Register(reinterpret_cast<IUnknown*>(g_notifObject), &g_notificationCookie);
    if (FAILED(hr)) {
        Wh_Log(L"[Notif] Register failed: 0x%08X", hr);
        Notif_Release(g_notifObject); g_notifObject = nullptr;
        notifSvc->Release(); CoUninitialize(); return 0;
    }
    Wh_Log(L"[Notif] Registered, cookie=%lu", g_notificationCookie);

    MSG msg;
    while (!g_unloading) {
        DWORD w = MsgWaitForMultipleObjects(1, &g_notificationStopEvent, FALSE, INFINITE, QS_ALLINPUT);
        if (w == WAIT_OBJECT_0) break;
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    }

    if (g_notificationCookie) { notifSvc->Unregister(g_notificationCookie); g_notificationCookie = 0; }
    if (g_notifObject)        { Notif_Release(g_notifObject); g_notifObject = nullptr; }
    notifSvc->Release();
    CoUninitialize();
    return 0;
}

static void StartNotificationThread() {
    if (g_notificationThread) return;
    g_notificationStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_notificationThread    = CreateThread(nullptr, 0, NotificationThreadProc, nullptr, 0, nullptr);
    if (!g_notificationThread) {
        CloseHandle(g_notificationStopEvent); g_notificationStopEvent = nullptr;
    }
}

static void StopNotificationThread() {
    if (g_notificationStopEvent) SetEvent(g_notificationStopEvent);
    if (g_notificationThread) {
        // Pump sent messages while waiting so that if Wh_ModUninit is called from
        // the UI thread and the notification thread is mid-SendMessage, the sent
        // message can be delivered and the notification thread can then exit.
        // PeekMessage(PM_NOREMOVE) processes incoming sent messages without
        // consuming posted messages from the queue.
        DWORD result;
        do {
            result = MsgWaitForMultipleObjects(1, &g_notificationThread, FALSE, INFINITE, QS_SENDMESSAGE);
            if (result == WAIT_OBJECT_0 + 1) {
                MSG msg;
                PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
            }
        } while (result == WAIT_OBJECT_0 + 1);
        CloseHandle(g_notificationThread); g_notificationThread = nullptr;
    }
    if (g_notificationStopEvent) {
        CloseHandle(g_notificationStopEvent); g_notificationStopEvent = nullptr;
    }
}

static void StopRetryThread() {
    if (g_retryStopEvent) SetEvent(g_retryStopEvent);
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
        CloseHandle(g_retryThread); g_retryThread = nullptr;
    }
    if (g_retryStopEvent) {
        CloseHandle(g_retryStopEvent); g_retryStopEvent = nullptr;
    }
}

// ============================================================
// Desktop state — registry
// ============================================================

static std::vector<BYTE> ReadRegBinary(const wchar_t* path, const wchar_t* name) {
    DWORD type = 0, size = 0;
    if (RegGetValueW(HKEY_CURRENT_USER, path, name, RRF_RT_REG_BINARY, &type, nullptr, &size) != ERROR_SUCCESS || !size)
        return {};
    std::vector<BYTE> buf(size);
    if (RegGetValueW(HKEY_CURRENT_USER, path, name, RRF_RT_REG_BINARY, &type, buf.data(), &size) != ERROR_SUCCESS)
        return {};
    buf.resize(size);
    return buf;
}

static int ReadDesktopCount() {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    wchar_t sessionPath[256];
    swprintf_s(sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\VirtualDesktops", sessionId);
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        auto buf = ReadRegBinary(path, L"VirtualDesktopIDs");
        if (buf.size() >= 16) return (int)(buf.size() / 16);
    }
    return 1;
}

static int ReadCurrentDesktop() {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    wchar_t sessionPath[256];
    swprintf_s(sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\VirtualDesktops", sessionId);

    std::vector<BYTE> ids;
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        ids = ReadRegBinary(path, L"VirtualDesktopIDs");
        if (ids.size() >= 16) break;
    }
    if (ids.empty()) return 0;

    GUID currentGuid{};
    bool gotCurrent = false;
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        auto buf = ReadRegBinary(path, L"CurrentVirtualDesktop");
        if (buf.size() >= 16) { memcpy(&currentGuid, buf.data(), 16); gotCurrent = true; break; }
        // Try REG_SZ form
        wchar_t strBuf[64]; DWORD sz = sizeof(strBuf), type;
        if (RegGetValueW(HKEY_CURRENT_USER, path, L"CurrentVirtualDesktop",
                         RRF_RT_REG_SZ, &type, strBuf, &sz) == ERROR_SUCCESS &&
            SUCCEEDED(CLSIDFromString(strBuf, &currentGuid))) { gotCurrent = true; break; }
    }
    if (!gotCurrent) return 0;

    int count = (int)(ids.size() / 16);
    for (int i = 0; i < count; i++) {
        GUID g; memcpy(&g, ids.data() + i * 16, 16);
        if (memcmp(&g, &currentGuid, 16) == 0) return i;
    }
    return 0;
}

// Read Windows display names for all desktops (registry Desktops\{GUID}\Name).
// Falls back to "Desktop N" when a desktop has no custom name.
static std::vector<std::wstring> ReadDesktopNames(int count) {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    wchar_t sessionPath[256];
    swprintf_s(sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%lu\\VirtualDesktops", sessionId);

    std::vector<BYTE> ids;
    for (auto* path : { (const wchar_t*)sessionPath, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops" }) {
        ids = ReadRegBinary(path, L"VirtualDesktopIDs");
        if (ids.size() >= 16) break;
    }

    std::vector<std::wstring> names(count);
    for (int i = 0; i < count; i++) {
        names[i] = L"Desktop " + std::to_wstring(i + 1);
        if ((int)ids.size() >= (i + 1) * 16) {
            GUID g; memcpy(&g, ids.data() + i * 16, 16);
            wchar_t guidStr[64];
            StringFromGUID2(g, guidStr, ARRAYSIZE(guidStr));
            wchar_t regPath[300];
            swprintf_s(regPath, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops\\Desktops\\%ls", guidStr);
            wchar_t name[256]; DWORD sz = sizeof(name);
            if (RegGetValueW(HKEY_CURRENT_USER, regPath, L"Name", RRF_RT_REG_SZ, nullptr, name, &sz) == ERROR_SUCCESS && name[0])
                names[i] = name;
        }
    }
    return names;
}

// ============================================================
// Virtual desktop switching
// ============================================================

struct IVirtualDesktopManagerInternal_S : IUnknown {};
struct IVirtualDesktop_S : IUnknown {};

MIDL_INTERFACE("92CA9DCD-5622-4bba-A805-5E9F541BD8C9")
IObjectArray_Local : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetCount(UINT* pcObjects) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAt(UINT i, REFIID riid, void** ppv) = 0;
};

const CLSID CLSID_VirtualDesktopManagerInternal = {
    0xC5E0CDCA,0x7B6E,0x41B2,{0x9F,0xC4,0xD9,0x39,0x75,0xCC,0x46,0x7B}
};

void SwitchToDesktop(int targetIndex) {
    if (!LoadTwinuiBuild()) { Wh_Log(L"[VD] twinui.pcshell.dll not loaded"); return; }

    IID IID_VDMI, IID_VD;
    bool usesHMonitor;
    if      (g_twinuiBuild >= 26100) {
        IID_VDMI = {0x53F5CA0B,0x158F,0x4124,{0x90,0x0C,0x05,0x71,0x58,0x06,0x0B,0x27}};
        IID_VD   = {0x3F07F4BE,0xB107,0x441A,{0xAF,0x0F,0x39,0xD8,0x25,0x29,0x07,0x2C}};
        usesHMonitor = false;
    } else if (g_twinuiBuild >= 22621) {
        IID_VDMI = {0xA3175F2D,0x239C,0x4BD2,{0x8A,0xA0,0xEE,0xBA,0x8B,0x0B,0x13,0x8E}};
        IID_VD   = {0x3F07F4BE,0xB107,0x441A,{0xAF,0x0F,0x39,0xD8,0x25,0x29,0x07,0x2C}};
        usesHMonitor = false;
    } else if (g_twinuiBuild >= 22000) {
        IID_VDMI = {0xB2F925B9,0x5A0F,0x4D2E,{0x9F,0x4D,0x2B,0x15,0x07,0x59,0x3C,0x10}};
        IID_VD   = {0x536D3495,0xB208,0x4CC9,{0xAE,0x26,0xDE,0x81,0x11,0x27,0x5B,0xF8}};
        usesHMonitor = true;
    } else if (g_twinuiBuild >= 20348) {
        IID_VDMI = {0x094AFE11,0x44F2,0x4BA0,{0x97,0x6F,0x29,0xA9,0x7E,0x26,0x3E,0xE0}};
        IID_VD   = {0x62FDF88B,0x11CA,0x4AFB,{0x8B,0xD8,0x22,0x96,0xDF,0xAE,0x49,0xE2}};
        usesHMonitor = true;
    } else {
        IID_VDMI = {0xF31574D6,0xB682,0x4CDC,{0xBD,0x56,0x18,0x27,0x86,0x0A,0xBE,0xC6}};
        IID_VD   = {0xFF72FFDD,0xBE7E,0x43FC,{0x9C,0x03,0xAD,0x81,0x68,0x1E,0x88,0xE4}};
        usesHMonitor = false;
    }

    IServiceProvider* svc = nullptr;
    if (FAILED(CoCreateInstance(CLSID_ImmersiveShell, nullptr, CLSCTX_LOCAL_SERVER,
                                IID_IServiceProvider, (void**)&svc)) || !svc)
        { Wh_Log(L"[VD] CoCreateInstance failed"); return; }

    IVirtualDesktopManagerInternal_S* mgr = nullptr;
    svc->QueryService(CLSID_VirtualDesktopManagerInternal, IID_VDMI, (void**)&mgr);
    svc->Release();
    if (!mgr) { Wh_Log(L"[VD] QueryService VDMI failed"); return; }

    IObjectArray_Local* arr = nullptr;
    if (usesHMonitor) {
        typedef HRESULT(STDMETHODCALLTYPE* FnM)(IVirtualDesktopManagerInternal_S*, HMONITOR, IObjectArray_Local**);
        ((FnM)(*(void***)mgr)[7])(mgr, nullptr, &arr);
    } else {
        typedef HRESULT(STDMETHODCALLTYPE* Fn)(IVirtualDesktopManagerInternal_S*, IObjectArray_Local**);
        ((Fn)(*(void***)mgr)[7])(mgr, &arr);
    }
    if (!arr) { mgr->Release(); Wh_Log(L"[VD] GetDesktops failed"); return; }

    UINT count = 0;
    arr->GetCount(&count);
    if (targetIndex < 0 || (UINT)targetIndex >= count) { arr->Release(); mgr->Release(); return; }

    IVirtualDesktop_S* target = nullptr;
    arr->GetAt((UINT)targetIndex, IID_VD, (void**)&target);
    arr->Release();
    if (!target) { mgr->Release(); Wh_Log(L"[VD] GetAt failed"); return; }

    if (usesHMonitor) {
        typedef HRESULT(STDMETHODCALLTYPE* FnM)(IVirtualDesktopManagerInternal_S*, HMONITOR, IVirtualDesktop_S*);
        ((FnM)(*(void***)mgr)[9])(mgr, nullptr, target);
    } else {
        typedef HRESULT(STDMETHODCALLTYPE* Fn)(IVirtualDesktopManagerInternal_S*, IVirtualDesktop_S*);
        ((Fn)(*(void***)mgr)[9])(mgr, target);
    }
    target->Release();
    mgr->Release();
    Wh_Log(L"[VD] Switched to desktop %d", targetIndex);
}

// ============================================================
// Unified element placement -- nested-group-layout template v2.3
// Copy-source: _templates/nested-group-layout.h. One expression, one
// setting: Layout.Arrangement is either the word "auto" (this file picks
// the shape and emits the expression, which the mod logs) or an explicit
// expression. Per-item offsets ride in that same string as "1[+2,-1]".
// ============================================================

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

// How many item rows fit in the taskbar. Pitch is one item plus one gap; the
// trailing gap of the last row is not required, hence the + spacing.
inline int AvailableRows(double taskbarHeightPx, unsigned dpi,
                         double itemHeight, double spacing) {
    double heightDip = PixelsToDip(taskbarHeightPx, dpi);
    double pitch = itemHeight + std::max(0.0, spacing);
    if (pitch <= 0.0 || heightDip <= 0.0)
        return 1;
    return std::max(1, (int)((heightDip + std::max(0.0, spacing)) / pitch));
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
// Button grid building
// ============================================================

static Brush GetWindowsAccentBrush(
    winrt::Windows::UI::ViewManagement::UIColorType colorType) {
    try {
        winrt::Windows::UI::ViewManagement::UISettings uiSettings;
        auto color = uiSettings.GetColorValue(colorType);
        SolidColorBrush brush;
        brush.Color(color);
        return brush;
    } catch (...) {
        Wh_Log(L"[Color] Failed to read the Windows accent color");
        return nullptr;
    }
}

static Brush ParseColorBrush(const std::wstring& value) {
    using winrt::Windows::UI::ViewManagement::UIColorType;

    if (value.empty())
        return nullptr;

    if (_wcsicmp(value.c_str(), L"transparent") == 0) {
        SolidColorBrush brush;
        brush.Color(winrt::Windows::UI::Color{0, 0, 0, 0});
        return brush;
    }

    // Numbered Windows shades are accepted silently and stay undocumented.
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
    for (auto const& entry : kAccentTokens)
        if (_wcsicmp(value.c_str(), entry.token) == 0)
            return GetWindowsAccentBrush(entry.type);

    if (value[0] != L'#') return nullptr;
    std::wstring h = value.substr(1);
    if (h.size() == 6) h = L"FF" + h;
    if (h.size() != 8) return nullptr;
    UINT32 val = 0;
    for (wchar_t c : h) {
        val <<= 4;
        if      (c >= L'0' && c <= L'9') val |= (UINT32)(c - L'0');
        else if (c >= L'A' && c <= L'F') val |= (UINT32)(10 + c - L'A');
        else if (c >= L'a' && c <= L'f') val |= (UINT32)(10 + c - L'a');
        else return nullptr;
    }
    winrt::Windows::UI::Color color;
    color.A = (BYTE)(val >> 24); color.R = (BYTE)(val >> 16);
    color.G = (BYTE)(val >> 8);  color.B = (BYTE)(val);
    SolidColorBrush brush; brush.Color(color); return brush;
}

static std::wstring ToRoman(int n) {
    if (n <= 0 || n > 3999) return std::to_wstring(n);
    static const struct { int v; const wchar_t* s; } t[] = {
        {1000,L"M"},{900,L"CM"},{500,L"D"},{400,L"CD"},
        {100,L"C"},{90,L"XC"},{50,L"L"},{40,L"XL"},
        {10,L"X"},{9,L"IX"},{5,L"V"},{4,L"IV"},{1,L"I"}
    };
    std::wstring r;
    for (auto& [v, s] : t) { while (n >= v) { r += s; n -= v; } }
    return r;
}

static std::wstring GetButtonLabel(int idx, int current) {
    if (g_settings.labelFormat == L"symbol")
        return (idx == current) ? g_settings.activeSymbol
                                : g_settings.inactiveSymbol;
    if (g_settings.labelFormat == L"roman")
        return ToRoman(idx + 1);
    if (g_settings.labelFormat == L"custom" && !g_settings.customLabels.empty()) {
        std::wistringstream ss(g_settings.customLabels);
        std::wstring token; int i = 0;
        while (std::getline(ss, token, L',')) { if (i++ == idx) return token; }
    }
    return std::to_wstring(idx + 1);
}

// Apply shine gradient to a base brush. Returns brush unchanged if no base color or shine off.
static Brush MakeShineBrush(Brush base) {
    if (!g_settings.shineEffect) return base;
    auto solid = base ? base.try_as<SolidColorBrush>() : nullptr;
    if (!solid) return base;
    auto c = solid.Color();

    LinearGradientBrush b;
    b.StartPoint({0.5, 0.0});
    b.EndPoint({0.5, 1.0});

    // Top: semi-transparent white highlight
    GradientStop g0; winrt::Windows::UI::Color shine{180,255,255,255};
    g0.Color(shine); g0.Offset(0.0); b.GradientStops().Append(g0);

    // Upper-mid: base color lightened slightly
    GradientStop g1;
    winrt::Windows::UI::Color light{c.A,
        (BYTE)std::min(255, (int)c.R + 35),
        (BYTE)std::min(255, (int)c.G + 35),
        (BYTE)std::min(255, (int)c.B + 35)};
    g1.Color(light); g1.Offset(0.42); b.GradientStops().Append(g1);

    // Lower: base color
    GradientStop g2; g2.Color(c); g2.Offset(0.52); b.GradientStops().Append(g2);

    // Bottom: slightly darker
    GradientStop g3;
    winrt::Windows::UI::Color dark{c.A,
        (BYTE)(c.R * 7 / 10), (BYTE)(c.G * 7 / 10), (BYTE)(c.B * 7 / 10)};
    g3.Color(dark); g3.Offset(1.0); b.GradientStops().Append(g3);

    return b;
}

// ---- Layout glue: settings -> the one arranger ------------------------------

// Rows that fit in this taskbar. The rect is physical pixels and every XAML
// size is a DIP, so the conversion happens before the division -- mixing them
// is the bug flagged on PR #4855 and #4843.
static int AvailableRows(bool quiet = false) {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    RECT rect{};
    if (!hWnd || !GetWindowRect(hWnd, &rect)) {
        if (!quiet)
            Wh_Log(L"[Layout] No taskbar window - assuming a single row");
        return 1;
    }
    UINT dpi = GetDpiForWindow(hWnd);
    int rows = ngl::AvailableRows((double)(rect.bottom - rect.top), dpi,
                                  (double)g_settings.itemHeight,
                                  (double)g_settings.itemSpacing);
    if (!quiet) {
        Wh_Log(L"[Layout] taskbar %dpx at %udpi -> %d row(s) available",
               (int)(rect.bottom - rect.top), dpi, rows);
    }
    return rows;
}

static ngl::Config MakeLayoutConfig() {
    ngl::Config config;
    config.spacing = (double)g_settings.itemSpacing;
    config.justify = g_settings.justify == L"start" ? ngl::Justify::Start
                   : g_settings.justify == L"end"   ? ngl::Justify::End
                                                    : ngl::Justify::Center;
    config.padX = (double)g_settings.padX;
    config.padY = (double)g_settings.padY;
    return config;
}

static ngl::FillOrder LayoutFillOrder() {
    return g_settings.fillOrder == L"columns" ? ngl::FillOrder::Columns
                                              : ngl::FillOrder::Rows;
}

// This mod's token vocabulary. Desktops are dynamic, so they are numbered;
// "desktop2" is accepted as a readable alias for "2", and the Task View button
// is "master". Matching is case-insensitive. Tokens are identity, never the
// button's label -- renaming a desktop never invalidates an arrangement.
// Returns a 0-based desktop index, or -1 when the token is not a desktop.
static int DesktopIndexFromToken(std::wstring const& token, int count) {
    int number = ngl::TokenIndexWithPrefix(token, L"desktop");
    if (!number) {
        wchar_t* end = nullptr;
        long parsed = std::wcstol(token.c_str(), &end, 10);
        if (!end || *end || end == token.c_str())
            return -1;
        number = (int)parsed;
    }
    if (number < 1 || number > count)
        return -1;
    return number - 1;
}

// Size of a layout token. "master" collapses to empty when the Task View
// button is off; anything unrecognized collapses out too, so a stray name in a
// hand-written arrangement costs nothing.
// The Task View button answers to "master" and to the friendlier "taskview".
static bool IsMasterToken(std::wstring const& token) {
    return ngl::TokenIs(token, L"master") || ngl::TokenIs(token, L"taskview");
}

// "Last button in the grid": the Task View button is just another cell, sized
// and shaped like a desktop button, rather than a column or sliver alongside.
static bool TaskViewInGrid() {
    return g_settings.taskViewPlacement == L"inGrid";
}

static ngl::Size ResolveLayoutToken(std::wstring const& token, int count) {
    if (IsMasterToken(token)) {
        if (!g_settings.taskViewButton)
            return {};
        if (TaskViewInGrid())
            return {(double)g_settings.itemWidth, (double)g_settings.itemHeight};
        // Sized against whichever axis it lands on, so one pair of settings
        // covers a full-height column and a full-width sliver, wherever the
        // arrangement puts it. Span 0 means match the desktop buttons.
        return ngl::AlongAxis((double)g_settings.taskViewSize,
                              (double)g_settings.taskViewSpan);
    }
    if (DesktopIndexFromToken(token, count) >= 0)
        return {(double)g_settings.itemWidth, (double)g_settings.itemHeight};
    return {};
}

// Wrap the generated desktop grid with the Task View button in its configured
// place. Only the automatic arrangement consults this -- a hand-written
// arrangement positions "master" itself.
// The Task View token, carrying Size.TaskViewGap as a cosmetic offset away
// from the desktop buttons. Positive is always "further away", whichever side
// it sits on. Cosmetic on purpose: the group keeps its size, so a sliver can
// hang past the taskbar edge and show only its leading edge instead of the
// whole group growing and re-centering.
static std::wstring MasterToken() {
    int gap = g_settings.taskViewGap;
    if (!gap)
        return L"master";
    std::wstring const& where = g_settings.taskViewPlacement;
    int dx = 0, dy = 0;
    if (where == L"before")     dx = -gap;
    else if (where == L"above") dy = -gap;
    else if (where == L"below") dy = gap;
    else                        dx = gap;  // "after"
    return L"master[" + std::to_wstring(dx) + L"," + std::to_wstring(dy) + L"]";
}

static std::wstring AddTaskViewButton(std::wstring const& grid) {
    if (!g_settings.taskViewButton || TaskViewInGrid())
        return grid;
    std::wstring const& where = g_settings.taskViewPlacement;
    std::wstring master = MasterToken();
    if (where == L"before")
        return master + L" | (" + grid + L")";
    if (where == L"above")
        return master + L", (" + grid + L")";
    if (where == L"below")
        return L"(" + grid + L"), " + master;
    return L"(" + grid + L") | " + master;  // "after"
}

// Every token this mod expects to be on screen right now.
static std::vector<std::wstring> ExpectedTokens(int count) {
    std::vector<std::wstring> tokens;
    for (int i = 1; i <= count; i++)
        tokens.push_back(std::to_wstring(i));
    if (g_settings.taskViewButton)
        tokens.push_back(L"master");
    return tokens;
}

// Resolve Layout.Arrangement to placements. "auto" generates the expression
// and logs it so it can be pasted back into the same field and edited; an
// explicit arrangement is used as written, falling back to automatic if it
// does not parse. A desktop created after the arrangement was written is
// appended (or left out) per Layout.NewItems, so a new desktop is never
// silently unreachable. quiet suppresses the log for repeated measure-only
// calls from the Start-placement layout callback.
static bool ComputeButtonPlacements(int count,
                                    std::vector<ngl::Placement>& placements,
                                    ngl::Size& total, bool quiet = false) {
    ngl::Config config = MakeLayoutConfig();
    auto resolve = [count](std::wstring const& token) {
        return ResolveLayoutToken(token, count);
    };

    bool isAuto = ngl::IsAutoSetting(g_settings.arrangement);
    int maxRows = AvailableRows(quiet);
    auto makeAuto = [count, maxRows]() {
        // In-grid mode shapes count + 1 cells so the Task View button flows
        // with the desktops instead of hanging off the side.
        bool inGrid = g_settings.taskViewButton && TaskViewInGrid();
        auto namer = [count](int index) -> std::wstring {
            return index < count ? std::to_wstring(index + 1)
                                 : std::wstring(L"master");
        };
        std::wstring grid = ngl::BuildAutoExpression(
            inGrid ? count + 1 : count, maxRows, LayoutFillOrder(), namer);
        if (grid.empty())
            grid = L"1";
        return AddTaskViewButton(grid);
    };

    std::wstring expression = isAuto ? makeAuto() : g_settings.arrangement;
    ngl::ParseError error;
    bool ok = ngl::Compute(expression, config, resolve, placements, total,
                           &error);
    if (!ok) {
        Wh_Log(L"[Layout] Arrangement \"%ls\" - expected %ls at character %d; "
               L"using the automatic arrangement instead",
               expression.c_str(), error.expected.c_str(),
               (int)error.position + 1);
        isAuto = true;
        expression = makeAuto();
        ok = ngl::Compute(expression, config, resolve, placements, total,
                          nullptr);
    }

    // A hand-written arrangement names the desktops that existed when it was
    // written. Anything created since is in no group and would vanish.
    // Compare by item identity, not by the name typed: "desktop1" and "1" are
    // the same button, and a plain name comparison would append a duplicate of
    // every aliased desktop.
    if (ok && !isAuto && g_settings.newItems != L"ignore") {
        auto sameItem = [count](std::wstring const& placed,
                                std::wstring const& wanted) {
            if (IsMasterToken(wanted))
                return IsMasterToken(placed);
            int a = DesktopIndexFromToken(placed, count);
            return a >= 0 && a == DesktopIndexFromToken(wanted, count);
        };
        auto missing = ngl::MissingTokens(ExpectedTokens(count), placements,
                                          sameItem);

        // Desktops and the Task View button are appended differently. Unlisted
        // desktops join a grid block after what the user wrote. The Task View
        // button instead honours its placement setting, so "before" and "above"
        // actually put it there — and so it never lands inside the desktop
        // block, where it would be squashed into a single cell instead of
        // staying a column or a sliver.
        std::vector<std::wstring> missingDesktops;
        bool missingMaster = false;
        for (auto const& token : missing) {
            if (IsMasterToken(token))
                missingMaster = true;
            else
                missingDesktops.push_back(token);
        }

        if (!missingDesktops.empty() || missingMaster) {
            if (TaskViewInGrid() && missingMaster)
                missingDesktops.push_back(L"master");
            if (!missingDesktops.empty()) {
                expression = ngl::AppendMissing(expression, missingDesktops,
                                                maxRows, LayoutFillOrder());
            }
            if (missingMaster && !TaskViewInGrid())
                expression = AddTaskViewButton(expression);
            ok = ngl::Compute(expression, config, resolve, placements, total,
                              nullptr);
            if (!quiet) {
                Wh_Log(L"[Layout] %d item(s) missing from your arrangement were "
                       L"added; name them in Arrangement to place them "
                       L"yourself", (int)missing.size());
            }
        }
    }

    if (!quiet) {
        Wh_Log(L"[Layout] %d desktop(s), arrangement = \"%ls\"%ls, size %.0fx%.0f",
               count, expression.c_str(),
               isAuto ? L" (auto - paste this into Arrangement to edit it)" : L"",
               total.width, total.height);
    }
    return ok;
}

// Measure only -- called from the Start-placement layout callback, so it must
// not log on every layout pass.
static ngl::Size EstimateButtonGridSize(int count) {
    std::vector<ngl::Placement> placements;
    ngl::Size total;
    ComputeButtonPlacements(count, placements, total, /*quiet=*/true);
    return total;
}

static void SetControlBrushResource(Control const& control,
                                    wchar_t const* key,
                                    Brush const& brush) {
    auto resources = control.Resources();
    auto boxedKey = winrt::box_value(key);
    if (brush)
        resources.Insert(boxedKey, brush);
    else
        resources.Remove(boxedKey);
}

static Brush AdjustSolidBrush(Brush const& base, double whiteBlend) {
    auto solid = base ? base.try_as<SolidColorBrush>() : nullptr;
    if (!solid)
        return nullptr;

    auto color = solid.Color();
    auto blend = [whiteBlend](BYTE channel) {
        double target = whiteBlend >= 0.0 ? 255.0 : 0.0;
        double amount = std::abs(whiteBlend);
        return (BYTE)std::clamp(
            (int)std::lround(channel + (target - channel) * amount), 0, 255);
    };

    winrt::Windows::UI::Color adjustedColor{
        color.A, blend(color.R), blend(color.G), blend(color.B)};
    SolidColorBrush adjusted;
    adjusted.Color(adjustedColor);
    return adjusted;
}

struct ButtonSurfaceBrushes {
    Brush normal;
    Brush hover;
    Brush pressed;
};

static ButtonSurfaceBrushes ResolveButtonSurface(
    bool isActive,
    Brush const& activeBrush,
    Brush const& inactiveBrush,
    Brush const& hoverOverride,
    Brush const& pressedOverride) {
    Brush normal = isActive ? activeBrush : inactiveBrush;

    // A native inactive surface keeps the native XAML hover/pressed states.
    // Colored surfaces derive their states from their own color unless the user
    // supplied an explicit shared override.
    Brush hover = hoverOverride;
    if (!hover && normal)
        hover = AdjustSolidBrush(normal, 0.18);

    Brush pressed = pressedOverride;
    if (!pressed && normal)
        pressed = AdjustSolidBrush(normal, -0.12);

    return {
        MakeShineBrush(normal),
        MakeShineBrush(hover),
        MakeShineBrush(pressed),
    };
}

static void ApplyMasterButtonState(Button const& btn,
    Brush inactiveBrush, Brush inactiveTextBrush,
    Brush hoverBrush, Brush pressedBrush, Brush borderBrush) {
    auto surface = ResolveButtonSurface(false, nullptr, inactiveBrush,
                                        hoverBrush, pressedBrush);
    if (surface.normal)
        btn.Background(surface.normal);
    else
        btn.ClearValue(Control::BackgroundProperty());
    SetControlBrushResource(btn, L"ButtonBackground", surface.normal);
    SetControlBrushResource(btn, L"ButtonBackgroundPointerOver", surface.hover);
    SetControlBrushResource(btn, L"ButtonBackgroundPressed", surface.pressed);

    if (inactiveTextBrush)
        btn.Foreground(inactiveTextBrush);
    else
        btn.ClearValue(Control::ForegroundProperty());
    SetControlBrushResource(btn, L"ButtonForeground", inactiveTextBrush);
    SetControlBrushResource(btn, L"ButtonForegroundPointerOver", inactiveTextBrush);
    SetControlBrushResource(btn, L"ButtonForegroundPressed", inactiveTextBrush);

    if (borderBrush)
        btn.BorderBrush(borderBrush);
    else
        btn.ClearValue(Control::BorderBrushProperty());
    SetControlBrushResource(btn, L"ButtonBorderBrush", borderBrush);
    SetControlBrushResource(btn, L"ButtonBorderBrushPointerOver", borderBrush);
    SetControlBrushResource(btn, L"ButtonBorderBrushPressed", borderBrush);
}

static void ApplyDesktopButtonState(ToggleButton const& btn, bool isActive,
    Brush activeBrush, Brush inactiveBrush,
    Brush activeTextBrush, Brush inactiveTextBrush,
    Brush hoverBrush, Brush pressedBrush, Brush borderBrush) {
    auto inactiveSurface = ResolveButtonSurface(false, activeBrush, inactiveBrush,
                                                hoverBrush, pressedBrush);
    auto activeSurface = ResolveButtonSurface(true, activeBrush, inactiveBrush,
                                              hoverBrush, pressedBrush);

    if (inactiveSurface.normal)
        btn.Background(inactiveSurface.normal);
    else
        btn.ClearValue(Control::BackgroundProperty());
    SetControlBrushResource(btn, L"ToggleButtonBackground", inactiveSurface.normal);
    SetControlBrushResource(btn, L"ToggleButtonBackgroundPointerOver", inactiveSurface.hover);
    SetControlBrushResource(btn, L"ToggleButtonBackgroundPressed", inactiveSurface.pressed);
    SetControlBrushResource(btn, L"ToggleButtonBackgroundChecked", activeSurface.normal);
    SetControlBrushResource(btn, L"ToggleButtonBackgroundCheckedPointerOver", activeSurface.hover);
    SetControlBrushResource(btn, L"ToggleButtonBackgroundCheckedPressed", activeSurface.pressed);

    if (inactiveTextBrush)
        btn.Foreground(inactiveTextBrush);
    else
        btn.ClearValue(Control::ForegroundProperty());
    SetControlBrushResource(btn, L"ToggleButtonForeground", inactiveTextBrush);
    SetControlBrushResource(btn, L"ToggleButtonForegroundPointerOver", inactiveTextBrush);
    SetControlBrushResource(btn, L"ToggleButtonForegroundPressed", inactiveTextBrush);
    SetControlBrushResource(btn, L"ToggleButtonForegroundChecked", activeTextBrush);
    SetControlBrushResource(btn, L"ToggleButtonForegroundCheckedPointerOver", activeTextBrush);
    SetControlBrushResource(btn, L"ToggleButtonForegroundCheckedPressed", activeTextBrush);

    if (borderBrush)
        btn.BorderBrush(borderBrush);
    else
        btn.ClearValue(Control::BorderBrushProperty());
    SetControlBrushResource(btn, L"ToggleButtonBorderBrush", borderBrush);
    SetControlBrushResource(btn, L"ToggleButtonBorderBrushPointerOver", borderBrush);
    SetControlBrushResource(btn, L"ToggleButtonBorderBrushPressed", borderBrush);
    SetControlBrushResource(btn, L"ToggleButtonBorderBrushChecked", borderBrush);
    SetControlBrushResource(btn, L"ToggleButtonBorderBrushCheckedPointerOver", borderBrush);
    SetControlBrushResource(btn, L"ToggleButtonBorderBrushCheckedPressed", borderBrush);

    if (g_settings.activeBold)
        btn.FontWeight(isActive
            ? winrt::Windows::UI::Text::FontWeights::Bold()
            : winrt::Windows::UI::Text::FontWeights::Normal());

    btn.IsThreeState(false);
    btn.IsChecked(isActive);
}

static void StyleButtonGeometry(Control const& btn,
                                std::wstring const& fontFamily) {
    btn.MinWidth(0.0);
    btn.MinHeight(0.0);
    btn.Padding({ 1.0, 0.0, 1.0, 0.0 });
    btn.FontSize((double)g_settings.fontSize);
    if (!fontFamily.empty())
        btn.FontFamily(winrt::Windows::UI::Xaml::Media::FontFamily(fontFamily));
    else
        btn.ClearValue(Control::FontFamilyProperty());
    btn.HorizontalAlignment(HorizontalAlignment::Stretch);
    btn.VerticalAlignment(VerticalAlignment::Stretch);

    double r = (double)g_settings.cornerRadius;
    btn.CornerRadius({ r, r, r, r });

    if (g_settings.borderThickness >= 0) {
        double t = (double)g_settings.borderThickness;
        btn.BorderThickness({ t, t, t, t });
    }
}

static void StyleDesktopButton(ToggleButton& btn, bool isActive,
    Brush activeBrush, Brush inactiveBrush,
    Brush activeTextBrush, Brush inactiveTextBrush,
    Brush hoverBrush, Brush pressedBrush, Brush borderBrush)
{
    StyleButtonGeometry(btn, g_settings.fontFamily);
    ApplyDesktopButtonState(btn, isActive, activeBrush, inactiveBrush,
                            activeTextBrush, inactiveTextBrush,
                            hoverBrush, pressedBrush, borderBrush);
}

static void StyleMasterButton(Button& btn,
    Brush inactiveBrush, Brush inactiveTextBrush,
    Brush hoverBrush, Brush pressedBrush, Brush borderBrush)
{
    StyleButtonGeometry(btn, g_settings.taskViewFontFamily);
    ApplyMasterButtonState(btn, inactiveBrush, inactiveTextBrush,
                           hoverBrush, pressedBrush, borderBrush);
}

static Grid BuildButtonGrid(int count, int current) {
    std::vector<ngl::Placement> placements;
    ngl::Size total;
    ComputeButtonPlacements(count, placements, total);

    Grid grid;
    std::vector<ButtonEventState> eventStates;
    grid.Name(L"VdSwitcherBar");
    // Absolute placement: one implicit cell, each button positioned by Margin
    // from the group's top-left. The group is sized to the arranger's padded
    // box and centered in the tray slot; spacing, shape, per-item offsets, and
    // short-group justification are already baked into the placements.
    grid.Width(total.width);
    grid.Height(total.height);
    grid.HorizontalAlignment(HorizontalAlignment::Center);
    grid.VerticalAlignment(VerticalAlignment::Center);
    // Adjust.OffsetX/Y translate the whole group without reserving space.
    if (g_settings.offsetX != 0 || g_settings.offsetY != 0)
        grid.Margin({(double)g_settings.offsetX, (double)g_settings.offsetY,
                     0.0, 0.0});
    if (g_settings.opacity < 100)
        grid.Opacity(std::max(0.0, std::min(1.0, g_settings.opacity / 100.0)));

    auto activeBrush       = ParseColorBrush(g_settings.activeBackgroundColor);
    auto inactiveBrush     = ParseColorBrush(g_settings.inactiveBackgroundColor);
    auto activeTextBrush   = ParseColorBrush(g_settings.activeTextColor);
    auto inactiveTextBrush = ParseColorBrush(g_settings.inactiveTextColor);
    auto hoverBrush        = ParseColorBrush(g_settings.hoverBackgroundColor);
    auto pressedBrush      = ParseColorBrush(g_settings.pressedBackgroundColor);
    auto borderBrush       = ParseColorBrush(g_settings.borderColor);
    auto desktopNames      = ReadDesktopNames(count);

    // Which token is which button, so a hand-written arrangement never has to
    // depend on the labels themselves.
    {
        std::wstring map;
        for (int i = 0; i < count; i++) {
            if (!map.empty())
                map += L"  ";
            map += std::to_wstring(i + 1) + L"=" + desktopNames[i];
        }
        if (g_settings.taskViewButton)
            map += L"  master=Task View";
        Wh_Log(L"[Layout] tokens: %ls", map.c_str());
    }

    for (auto const& p : placements) {
        if (IsMasterToken(p.token)) {
            Button masterBtn;
            masterBtn.Name(L"VdMasterBtn");
            TextBlock content;
            content.Text(winrt::hstring(g_settings.taskViewLabel));
            content.HorizontalAlignment(HorizontalAlignment::Center);
            content.VerticalAlignment(VerticalAlignment::Center);
            masterBtn.Content(content);
            StyleMasterButton(masterBtn, inactiveBrush, inactiveTextBrush,
                              hoverBrush, pressedBrush, borderBrush);
            masterBtn.Width(p.size.width);
            masterBtn.Height(p.size.height);
            masterBtn.HorizontalAlignment(HorizontalAlignment::Left);
            masterBtn.VerticalAlignment(VerticalAlignment::Top);
            masterBtn.Margin({p.x, p.y, 0.0, 0.0});
            ToolTipService::SetToolTip(masterBtn,
                winrt::box_value(winrt::hstring(L"Task View (Win+Tab)")));
            auto masterClickToken = masterBtn.Click([](auto const&, auto const&) {
                if (g_unloading) return;
                INPUT inputs[4]{};
                inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_LWIN;
                inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = VK_TAB;
                inputs[2].type = INPUT_KEYBOARD; inputs[2].ki.wVk = VK_TAB;  inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
                inputs[3].type = INPUT_KEYBOARD; inputs[3].ki.wVk = VK_LWIN; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
            });
            eventStates.push_back({grid, masterBtn, masterClickToken});
            grid.Children().Append(masterBtn);
            continue;
        }

        int idx = DesktopIndexFromToken(p.token, count);
        if (idx < 0)
            continue;

        ToggleButton btn;
        btn.Name(L"VdBtn_" + std::to_wstring(idx));
        TextBlock content;
        content.Text(winrt::hstring(GetButtonLabel(idx, current)));
        content.HorizontalAlignment(HorizontalAlignment::Center);
        content.VerticalAlignment(VerticalAlignment::Center);
        btn.Content(content);
        StyleDesktopButton(btn, idx == current, activeBrush, inactiveBrush,
                           activeTextBrush, inactiveTextBrush,
                           hoverBrush, pressedBrush, borderBrush);
        btn.Width(p.size.width);
        btn.Height(p.size.height);
        btn.HorizontalAlignment(HorizontalAlignment::Left);
        btn.VerticalAlignment(VerticalAlignment::Top);
        btn.Margin({p.x, p.y, 0.0, 0.0});
        ToolTipService::SetToolTip(btn, winrt::box_value(winrt::hstring(desktopNames[idx])));

        int capturedIdx = idx;
        auto clickToken = btn.Click([capturedIdx](auto const& sender, auto const&) {
            // ToggleButton changes IsChecked before raising Click. Keep the
            // visual state tied to the actual current desktop while the COM
            // switch runs asynchronously (and if the switch fails).
            try {
                if (auto toggle = sender.template try_as<ToggleButton>())
                    toggle.IsChecked(capturedIdx == g_currentDesktop.load());
            } catch (...) {
                LogCurrentUiException(L"desktop button click");
            }
            if (g_unloading) return;
            // Dispatch to a background thread to avoid STA re-entrancy: when
            // SwitchToDesktop makes a LOCAL_SERVER COM call on the UI thread,
            // the STA message pump runs and can deliver the notification thread's
            // SendMessage re-entrantly, corrupting XAML state mid-click.
            g_activeSwitchThreads.fetch_add(1);
            HANDLE h = CreateThread(nullptr, 0, [](LPVOID p2) -> DWORD {
                int i2 = (int)(INT_PTR)p2;
                CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
                if (!g_unloading) SwitchToDesktop(i2);
                CoUninitialize();
                g_activeSwitchThreads.fetch_sub(1);
                return 0;
            }, (LPVOID)(INT_PTR)capturedIdx, 0, nullptr);
            if (h) CloseHandle(h);
            else g_activeSwitchThreads.fetch_sub(1);
        });
        eventStates.push_back({grid, btn, clickToken});
        grid.Children().Append(btn);
    }

    for (auto& state : eventStates)
        g_buttonEventStates->push_back(std::move(state));
    return grid;
}

// ============================================================
// Injection into XAML tree
// ============================================================

static bool PositionIsStartMode() {
    const auto& pos = g_settings.position;
    return pos == L"leftOfStart" || pos == L"overStart" ||
           pos == L"rightOfStart";
}

static FrameworkElement FindStartButton(FrameworkElement root) {
    return FindChildRecursive(root, [](FrameworkElement fe) {
        if (winrt::get_class_name(fe) != L"Taskbar.ExperienceToggleButton")
            return false;
        return AutomationProperties::GetAutomationId(fe) == L"StartButton";
    });
}

static FrameworkElement FindTaskbarRootGrid(FrameworkElement root) {
    auto taskbarFrame = FindChildRecursive(root, [](FrameworkElement fe) {
        return winrt::get_class_name(fe) == L"Taskbar.TaskbarFrame";
    });
    if (!taskbarFrame) return nullptr;

    int n = VisualTreeHelper::GetChildrenCount(taskbarFrame);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(taskbarFrame, i).try_as<FrameworkElement>();
        if (child && child.Name() == L"RootGrid")
            return child;
    }
    return nullptr;
}

static FrameworkElement FindTaskbarFrameRepeater(FrameworkElement rootGrid) {
    int n = VisualTreeHelper::GetChildrenCount(rootGrid);
    for (int i = 0; i < n; i++) {
        auto child = VisualTreeHelper::GetChild(rootGrid, i).try_as<FrameworkElement>();
        if (child && child.Name() == L"TaskbarFrameRepeater")
            return child;
    }
    return nullptr;
}

static void SetTaskItemsLeftMargin(double left) {
    if (!g_taskItemsPanel)
        return;

    auto margin = g_taskItemsPanel.Margin();
    if (std::fabs(margin.Left - left) <= 0.5)
        return;

    margin.Left = left;
    g_taskItemsPanel.Margin(margin);
}

static void SetStartButtonVisualOffset(double x) {
    if (!g_startOverlayStart)
        return;

    if (std::fabs(x) <= 0.5) {
        if (!g_startOverlayStart.RenderTransform())
            return;
        g_startOverlayStart.ClearValue(UIElement::RenderTransformProperty());
        return;
    }

    auto existing = g_startOverlayStart.RenderTransform().try_as<TranslateTransform>();
    if (existing && std::fabs(existing.X() - x) <= 0.5 && existing.Y() == 0.0)
        return;

    TranslateTransform tt;
    tt.X(x);
    tt.Y(0.0);
    g_startOverlayStart.RenderTransform(tt);
}

static double GetElementActualWidth(FrameworkElement const& element) {
    return element ? element.ActualWidth() : 0.0;
}

static double GetElementActualHeight(FrameworkElement const& element) {
    return element ? element.ActualHeight() : 0.0;
}

static void PositionButtonGridNearStart() {
    if (!g_buttonGrid || !g_startOverlayRoot || !g_startOverlayStart)
        return;

    int count = g_desktopCount.load();
    // The arranger's total already includes Adjust.PadX/PadY on both sides.
    ngl::Size gridSize = EstimateButtonGridSize(count);
    double gridW = gridSize.width;
    double gridH = gridSize.height;
    const auto& pos = g_settings.position;

    bool startHidden = (g_startOverlayStart.Visibility() == Visibility::Collapsed);
    double startW = g_startOverlayStart.ActualWidth();
    double startH = g_startOverlayStart.ActualHeight();
    if (startW <= 0.0 && !startHidden) startW = 44.0;
    if (startH <= 0.0) startH = std::max((double)g_settings.itemHeight, gridH);

    // x changes when TaskbarFrameRepeater.Margin.Left is pushed (start button moves with it).
    // y is unaffected by horizontal margin changes and is always valid for vertical centering.
    double x = 0.0;
    double y = 0.0;
    try {
        auto transform = g_startOverlayStart.TransformToVisual(g_startOverlayRoot);
        winrt::Windows::Foundation::Point origin{ 0.0f, 0.0f };
        auto p = transform.TransformPoint(origin);
        x = p.X;
        y = p.Y;
    } catch (...) {
    }

    double left = 0.0;
    double top  = 0.0;

    if (pos == L"overStart") {
        // Grid overlays the Start button. Adjust.OffsetY nudges it vertically.
        double anchorX = (g_startButtonOriginalX >= 0.0) ? g_startButtonOriginalX : x;
        left = anchorX;
        top  = y + (startH - gridH) / 2.0;
        if (left < 0.0) left = 0.0;
        SetStartButtonVisualOffset(0.0);
    } else if (pos == L"rightOfStart") {
        // Grid sits immediately right of the start button and reserves room for
        // itself before taskbar items. TaskbarFrameRepeater.Margin.Left moves
        // Start too, so counter-shift Start visually back to its stable anchor.
        double anchorX = (g_startButtonOriginalX >= 0.0) ? g_startButtonOriginalX : x;
        left = anchorX + startW;
        top  = y + (startH - gridH) / 2.0;
        if (left < 0.0) left = 0.0;

        if (g_taskItemsPanel) {
            double push = gridW + (double)g_settings.itemSpacing;
            SetTaskItemsLeftMargin(g_taskItemsPanelOriginalMargin.Left + push);
            if (!startHidden)
                SetStartButtonVisualOffset(-push);
        } else {
            SetStartButtonVisualOffset(0.0);
        }
    } else {
        // leftOfStart: anchor the grid at the left edge; push TaskbarFrameRepeater
        // rightward so Start button and task items don't overlap the grid.
        // y from TransformToVisual is unaffected by Margin.Left changes, so it
        // stays valid for vertical centering even after we push the panel right.
        left = 0.0;
        top  = y + (startH - gridH) / 2.0;
        SetStartButtonVisualOffset(0.0);

        if (g_taskItemsPanel) {
            double neededLeft = g_taskItemsPanelOriginalMargin.Left +
                                gridW + (double)g_settings.itemSpacing;
            SetTaskItemsLeftMargin(neededLeft);
        }
    }

    // Adjust.OffsetX/Y nudge the grid in every Start mode. In tray positions
    // BuildButtonGrid applies them as the group's margin instead.
    left += (double)g_settings.offsetX;
    top  += (double)g_settings.offsetY;

    double rootW = GetElementActualWidth(g_startOverlayRoot);
    double rootH = GetElementActualHeight(g_startOverlayRoot);
    if (rootW > 0.0 && left + gridW > rootW)
        left = std::max(0.0, rootW - gridW);
    if (rootH > 0.0 && top + gridH > rootH)
        top = std::max(0.0, rootH - gridH);
    if (top < 0.0) top = 0.0;

    g_buttonGrid.HorizontalAlignment(HorizontalAlignment::Left);
    g_buttonGrid.VerticalAlignment(VerticalAlignment::Top);
    auto current = g_buttonGrid.Margin();
    if (std::fabs(current.Left - left) > 0.5 ||
        std::fabs(current.Top - top) > 0.5 ||
        current.Right != 0.0 ||
        current.Bottom != 0.0) {
        g_buttonGrid.Margin({ left, top, 0.0, 0.0 });
    }
}

static bool InjectButtonGridNearStart(FrameworkElement root) {
    auto rootGrid = FindTaskbarRootGrid(root);
    if (!rootGrid) {
        Wh_Log(L"[Inject] Taskbar RootGrid not found");
        return false;
    }

    auto gridParent = rootGrid.try_as<Grid>();
    if (!gridParent) {
        Wh_Log(L"[Inject] Taskbar RootGrid is not a Grid");
        return false;
    }

    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"VdSwitcherBar")
            return true;
    }

    auto startButton = FindStartButton(root);
    if (!startButton) {
        Wh_Log(L"[Inject] StartButton not found");
        return false;
    }

    int count   = ReadDesktopCount();
    int current = ReadCurrentDesktop();
    g_desktopCount.store(count);
    g_currentDesktop.store(current);

    if (g_settings.hideWhenSingle && count <= 1) {
        Wh_Log(L"[Inject] Skipping start overlay - hideWhenSingle, count=%d", count);
        return true;
    }

    auto grid = BuildButtonGrid(count, current);
    grid.IsHitTestVisible(true);
    Grid::SetColumn(grid, 0);
    Grid::SetColumnSpan(grid, std::max(1, (int)gridParent.ColumnDefinitions().Size()));
    Canvas::SetZIndex(grid, 1000);
    gridParent.Children().Append(grid);

    g_buttonGrid = grid;
    g_injectionParent = rootGrid;
    g_injectedColumn = -1;
    g_startOverlayMode = true;
    g_startOverlayRoot = rootGrid;
    g_startOverlayStart = startButton;

    // Capture TaskbarFrameRepeater for modes that reserve space near Start.
    if (auto repeater = FindTaskbarFrameRepeater(rootGrid)) {
        g_taskItemsPanel = repeater;
        g_taskItemsPanelOriginalMargin = repeater.Margin();
    }

    // Capture original start button x before any margin pushes — stable anchor.
    try {
        auto t = startButton.TransformToVisual(rootGrid);
        winrt::Windows::Foundation::Point o{ 0.0f, 0.0f };
        g_startButtonOriginalX = t.TransformPoint(o).X;
    } catch (...) {
        g_startButtonOriginalX = -1.0;
    }

    PositionButtonGridNearStart();
    g_startOverlayLayoutToken = rootGrid.LayoutUpdated(
        [](winrt::Windows::Foundation::IInspectable const&, auto const&) {
            try {
                if (!g_unloading)
                    PositionButtonGridNearStart();
            } catch (...) {
                LogCurrentUiException(L"Start placement layout");
            }
        });

    Wh_Log(L"[Inject] VdSwitcherBar near Start (%d desktops, current=%d)", count, current);
    return true;
}

// Map g_settings.position to a target column in a SystemTrayFrameGrid, insert
// a new Auto-width column there, shift existing children to make room, and
// append the grid. Returns the column index the grid was placed in.
static int InsertGridIntoTrayColumns(Grid const& gridParent, Grid const& grid) {
    const auto& pos = g_settings.position;

    // Find a named direct child of the tray grid.
    auto findNamedDirect = [&](const wchar_t* name) -> FrameworkElement {
        for (auto child : gridParent.Children()) {
            if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == name)
                return fe;
        }
        return nullptr;
    };

    // Map position setting → reference element + whether to insert after it.
    // Secondary taskbars may lack some of these elements; a missing reference
    // falls through to column 0 (before icons).
    FrameworkElement refElem = nullptr;
    bool insertAfterRef = false;

    if      (pos == L"beforeOmni")
        refElem = findNamedDirect(L"ControlCenterButton");
    else if (pos == L"beforeClock")
        refElem = findNamedDirect(L"NotificationCenterButton");
    else if (pos == L"afterClock")
        refElem = findNamedDirect(L"ShowDesktopStack");
    else if (pos == L"afterShowDesktop") {
        refElem = findNamedDirect(L"ShowDesktopStack");
        insertAfterRef = true;
    }
    // beforeIcons → column 0 (refElem stays nullptr)

    int insertCol;
    if (insertAfterRef && refElem)
        insertCol = Grid::GetColumn(refElem) + 1;
    else if (refElem)
        insertCol = Grid::GetColumn(refElem);
    else
        insertCol = 0;  // beforeIcons: leftmost column in tray

    // Insert a new Auto-width column at insertCol.
    ColumnDefinition cd;
    cd.Width({ 1.0, GridUnitType::Auto });
    if ((uint32_t)insertCol < gridParent.ColumnDefinitions().Size())
        gridParent.ColumnDefinitions().InsertAt((uint32_t)insertCol, cd);
    else
        gridParent.ColumnDefinitions().Append(cd);

    // Shift every existing child whose column is >= insertCol to make room.
    // Elements that start before insertCol but span through it get their span
    // widened so they continue to cover the same original columns (plus the new one).
    for (auto child : gridParent.Children()) {
        auto fe = child.try_as<FrameworkElement>();
        if (!fe) continue;
        int col  = Grid::GetColumn(fe);
        int span = Grid::GetColumnSpan(fe);
        if (col >= insertCol)
            Grid::SetColumn(fe, col + 1);
        else if (col + span > insertCol)
            Grid::SetColumnSpan(fe, span + 1);
    }

    Grid::SetColumn(grid, insertCol);
    Canvas::SetZIndex(grid, 10000);
    gridParent.Children().Append(grid);
    return insertCol;
}

static bool InjectButtonGrid(FrameworkElement root) {
    if (PositionIsStartMode())
        return InjectButtonGridNearStart(root);

    FrameworkElement parent = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    if (!parent) {
        Wh_Log(L"[Inject] SystemTrayFrameGrid not found");
        return false;
    }

    // SystemTrayFrameGrid is a Grid with column-based layout. We must insert a new
    // ColumnDefinition and shift existing elements rather than relying on Children order.
    auto gridParent = parent.try_as<Grid>();
    if (!gridParent) { Wh_Log(L"[Inject] Parent is not a Grid"); return false; }

    // Already injected?
    for (auto child : gridParent.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"VdSwitcherBar") {
            // Re-acquire state in case it was lost (e.g., transient null from GetTaskbarXamlRoot
            // during RebuildButtonGrid caused g_buttonGrid to be cleared while grid stayed in tree).
            if (!g_buttonGrid) {
                g_buttonGrid = fe.try_as<Grid>();
                g_injectionParent = parent;
                g_injectedColumn = Grid::GetColumn(fe);
                Wh_Log(L"[Inject] Re-acquired existing VdSwitcherBar at col=%d", g_injectedColumn);
            }
            return true;
        }
    }

    int count   = ReadDesktopCount();
    int current = ReadCurrentDesktop();
    g_desktopCount.store(count);
    g_currentDesktop.store(current);

    if (g_settings.hideWhenSingle && count <= 1) {
        Wh_Log(L"[Inject] Skipping — hideWhenSingle, count=%d", count);
        return true;  // notification thread will watch for desktop additions
    }

    auto grid = BuildButtonGrid(count, current);
    int insertCol = InsertGridIntoTrayColumns(gridParent, grid);
    g_buttonGrid      = grid;
    g_injectionParent = parent;
    g_injectedColumn  = insertCol;

    Wh_Log(L"[Inject] VdSwitcherBar at column=%d in %ls (%d desktops, current=%d)",
           insertCol, parent.Name().c_str(), count, current);
    return true;
}

static Grid FindLiveSystemTrayFrameGrid() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return nullptr;

    auto xamlRoot = GetTaskbarXamlRoot(hWnd);
    if (!xamlRoot) return nullptr;
    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return nullptr;

    auto parent = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    });
    return parent ? parent.try_as<Grid>() : nullptr;
}

// XAML can defer teardown of a removed subtree until after the mod DLL unloads.
// Release every delegate and mod-created boxed value before detaching a grid.
static void ClearButtonEventState(Grid const& owner) {
    if (!owner) return;
    for (auto it = g_buttonEventStates->begin(); it != g_buttonEventStates->end();) {
        if (it->owner != owner) {
            ++it;
            continue;
        }
        try {
            if (it->clickToken.value)
                it->button.Click(it->clickToken);
            ToolTipService::SetToolTip(it->button, nullptr);
            if (auto contentControl = it->button.try_as<ContentControl>())
                contentControl.Content(nullptr);
        } catch (...) {
            Wh_Log(L"[Remove] Failed to clear one button's event state");
        }
        it = g_buttonEventStates->erase(it);
    }
}

static void ClearAllButtonEventState() {
    while (!g_buttonEventStates->empty()) {
        auto owner = g_buttonEventStates->front().owner;
        if (owner)
            ClearButtonEventState(owner);
        else
            g_buttonEventStates->erase(g_buttonEventStates->begin());
    }
}

static bool RemoveButtonGridFrom(Grid gridParent, int col) {
    if (!gridParent) return false;

    uint32_t removeIdx = (uint32_t)-1;
    int liveCol = col;
    for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
        auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
        if (fe && fe.Name() == L"VdSwitcherBar") {
            removeIdx = i;
            liveCol = Grid::GetColumn(fe);
            break;
        }
    }
    if (removeIdx == (uint32_t)-1) return false;

    auto ownedGrid = gridParent.Children().GetAt(removeIdx).try_as<Grid>();
    ClearButtonEventState(ownedGrid);
    gridParent.Children().RemoveAt(removeIdx);

    if (liveCol >= 0) {
        uint32_t colU = (uint32_t)liveCol;
        if (colU < gridParent.ColumnDefinitions().Size())
            gridParent.ColumnDefinitions().RemoveAt(colU);
        for (auto child : gridParent.Children()) {
            auto fe = child.try_as<FrameworkElement>();
            if (!fe) continue;
            int c    = Grid::GetColumn(fe);
            int span = Grid::GetColumnSpan(fe);
            if (c > liveCol)
                Grid::SetColumn(fe, c - 1);
            else if (c < liveCol && c + span > liveCol)
                Grid::SetColumnSpan(fe, span - 1);
        }
    }
    return true;
}

static void RemoveButtonGrid() {
    if (g_startOverlayMode) {
        if (g_startOverlayRoot && g_startOverlayLayoutToken.value) {
            g_startOverlayRoot.LayoutUpdated(g_startOverlayLayoutToken);
            g_startOverlayLayoutToken = {};
        }

        auto gridParent = g_injectionParent ? g_injectionParent.try_as<Grid>() : nullptr;
        bool removed = false;
        if (gridParent) {
            for (uint32_t i = 0; i < gridParent.Children().Size(); i++) {
                auto fe = gridParent.Children().GetAt(i).try_as<FrameworkElement>();
                if (fe && fe.Name() == L"VdSwitcherBar") {
                    ClearButtonEventState(fe.try_as<Grid>());
                    gridParent.Children().RemoveAt(i);
                    removed = true;
                    break;
                }
            }
        }
        if (!removed)
            ClearButtonEventState(g_buttonGrid);

        if (g_taskItemsPanel) {
            g_taskItemsPanel.Margin(g_taskItemsPanelOriginalMargin);
            g_taskItemsPanel = nullptr;
        }
        SetStartButtonVisualOffset(0.0);
        g_startButtonOriginalX = -1.0;

        g_buttonGrid = nullptr;
        g_injectionParent = nullptr;
        g_injectedColumn = -1;
        g_startOverlayMode = false;
        g_startOverlayRoot = nullptr;
        g_startOverlayStart = nullptr;
        return;
    }

    auto gridParent = FindLiveSystemTrayFrameGrid();
    if (!gridParent) {
        Wh_Log(L"[Remove] No live tray grid; retaining VdSwitcherBar state");
        return;
    }
    if (!RemoveButtonGridFrom(gridParent, g_injectedColumn)) {
        Wh_Log(L"[Remove] VdSwitcherBar not found");
        ClearButtonEventState(g_buttonGrid);
    }

    g_buttonGrid      = nullptr;
    g_injectionParent = nullptr;
    g_injectedColumn  = -1;
}

// ============================================================
// Secondary taskbars (experimental multi-monitor)
// ============================================================
// GetTaskbarXamlRoot goes through the primary CTaskBand only, so secondary
// taskbars (Shell_SecondaryTrayWnd) are discovered from IconView elements as
// they load: an element's XamlRoot belongs to whichever taskbar hosts it.
// Discovered tray grids stay registered while the toggle is off so enabling
// it can reinject without waiting for new icons. All access is UI-thread only.

struct SecondaryBar {
    Grid trayGrid{nullptr};    // SystemTrayFrameGrid of a secondary taskbar
    Grid buttonGrid{nullptr};  // injected VdSwitcherBar, null when not injected
};
[[clang::no_destroy]] static std::optional<std::vector<SecondaryBar>>
    g_secondaryBars{std::in_place};

static bool IsTrayGridAlive(Grid const& trayGrid) {
    try {
        return trayGrid && trayGrid.XamlRoot() != nullptr;
    } catch (...) {
        return false;
    }
}

static bool WantSecondaryBars() {
    return g_settings.allTaskbars && !PositionIsStartMode() &&
           !(g_settings.hideWhenSingle && g_desktopCount.load() <= 1);
}

// Remove injected grids from all secondary taskbars, keeping the tray-grid
// registrations so the bars can be reinjected without rediscovery.
static void RemoveSecondaryBars() {
    for (auto& bar : *g_secondaryBars) {
        if (bar.buttonGrid) {
            if (IsTrayGridAlive(bar.trayGrid))
                RemoveButtonGridFrom(bar.trayGrid, -1);
            else
                ClearButtonEventState(bar.buttonGrid);
        }
        bar.buttonGrid = nullptr;
    }
}

// Remove and reinject the grid on every known secondary taskbar (or just
// remove, when the toggle/settings no longer want them). Full rebuild for the
// same reason as the primary grid — see RebuildButtonGrid.
static void RefreshSecondaryBars() {
    if (g_secondaryBars->empty())
        return;
    int count    = g_desktopCount.load();
    int current  = g_currentDesktop.load();
    bool wantBars = WantSecondaryBars();
    for (auto it = g_secondaryBars->begin(); it != g_secondaryBars->end();) {
        auto& bar = *it;
        if (!IsTrayGridAlive(bar.trayGrid)) {
            // Taskbar went away (monitor disconnected or tray rebuilt); a new
            // IconView load on that taskbar will rediscover it.
            ClearButtonEventState(bar.buttonGrid);
            it = g_secondaryBars->erase(it);
            continue;
        }
        if (bar.buttonGrid) {
            RemoveButtonGridFrom(bar.trayGrid, -1);
            bar.buttonGrid = nullptr;
        }
        if (wantBars) {
            auto grid = BuildButtonGrid(count, current);
            InsertGridIntoTrayColumns(bar.trayGrid, grid);
            bar.buttonGrid = grid;
        }
        ++it;
    }
}

// Register the tray grid of the taskbar hosting this element and inject the
// switcher into it when the toggle is on. The primary taskbar is excluded —
// it is handled by the main injection path (including Start positions).
static void RegisterSecondaryTrayFromElement(FrameworkElement const& element) {
    if (g_unloading || !g_buttonGrid)  // wait until the primary bar exists
        return;

    auto xamlRoot = element.XamlRoot();
    if (!xamlRoot) return;
    auto root = xamlRoot.Content().try_as<FrameworkElement>();
    if (!root) return;

    // Never treat the primary taskbar as secondary.
    if (g_taskbarWnd) {
        try {
            if (auto primaryRoot = GetTaskbarXamlRoot(g_taskbarWnd))
                if (primaryRoot.Content() == xamlRoot.Content())
                    return;
        } catch (...) {
        }
    }

    auto trayGrid = FindChildRecursive(root, [](FrameworkElement fe) {
        return fe.Name() == L"SystemTrayFrameGrid";
    }).try_as<Grid>();
    if (!trayGrid) return;

    for (auto& bar : *g_secondaryBars)
        if (bar.trayGrid == trayGrid) return;  // already known

    // Defensive: skip trays that somehow already contain our bar.
    for (auto child : trayGrid.Children()) {
        if (auto fe = child.try_as<FrameworkElement>(); fe && fe.Name() == L"VdSwitcherBar")
            return;
    }

    SecondaryBar bar;
    bar.trayGrid = trayGrid;
    if (WantSecondaryBars()) {
        auto grid = BuildButtonGrid(g_desktopCount.load(), g_currentDesktop.load());
        InsertGridIntoTrayColumns(bar.trayGrid, grid);
        bar.buttonGrid = grid;
        Wh_Log(L"[Inject] VdSwitcherBar on secondary taskbar (%u registered)",
               (unsigned)(g_secondaryBars->size() + 1));
    }
    g_secondaryBars->push_back(std::move(bar));
}

// Rebuild the button grid on the UI thread. Always a full rebuild: button
// backgrounds use lightweight styling resources (ButtonBackground etc.), and
// {ThemeResource} references resolve once at template application — swapping
// the resources on a live button has no effect, so in-place highlight updates
// can't work. Freshly built buttons resolve correctly.
static void RebuildButtonGrid() {
    int count   = ReadDesktopCount();
    int current = ReadCurrentDesktop();
    g_desktopCount.store(count);
    g_currentDesktop.store(current);

    // Secondary bars first — RefreshSecondaryBars handles the hideWhenSingle
    // and toggle-off cases internally, so it must run before the early returns.
    RefreshSecondaryBars();

    if (g_settings.hideWhenSingle) {
        if (count <= 1) {
            if (g_buttonGrid) RemoveButtonGrid();
            return;
        }
        if (!g_buttonGrid) {
            ApplyAllSettings();
            return;
        }
    }

    if (!g_buttonGrid) { ApplyAllSettings(); return; }
    Grid gridParent{nullptr};
    uint32_t idx;
    if (g_startOverlayMode) {
        if (!g_injectionParent) { ApplyAllSettings(); return; }
        gridParent = g_injectionParent.try_as<Grid>();
        if (!gridParent || !gridParent.Children().IndexOf(g_buttonGrid, idx)) {
            ClearButtonEventState(g_buttonGrid);
            g_buttonGrid = nullptr;
            g_injectionParent = nullptr;
            g_injectedColumn = -1;
            ApplyAllSettings();
            return;
        }
    } else {
        // Use the live XAML tree — g_injectionParent may be stale if Windows
        // rebuilt the tray after a desktop add/remove.
        gridParent = FindLiveSystemTrayFrameGrid();
        if (!gridParent) {
            // XAML tree temporarily inaccessible (e.g., mid-rebuild by Windows).
            // Do NOT null g_buttonGrid — the grid is still in the tree; we just
            // can't reach it right now. The next notification will retry.
            Wh_Log(L"[Rebuild] XAML tree not accessible, deferring");
            return;
        }
        if (!gridParent.Children().IndexOf(g_buttonGrid, idx)) {
            // Our grid is genuinely gone (tray was rebuilt). Reinject from scratch.
            ClearButtonEventState(g_buttonGrid);
            g_buttonGrid = nullptr;
            g_injectionParent = nullptr;
            g_injectedColumn = -1;
            ApplyAllSettings();
            return;
        }
    }
    // Capture the live column BEFORE removing the old grid, so even if
    // g_injectedColumn is stale (columns were renumbered by Windows), we
    // reinsert at the correct position.
    int liveColumn = g_startOverlayMode ? 0 : Grid::GetColumn(g_buttonGrid);
    ClearButtonEventState(g_buttonGrid);
    gridParent.Children().RemoveAt(idx);
    g_buttonGrid = BuildButtonGrid(count, current);
    if (g_startOverlayMode) {
        Grid::SetColumn(g_buttonGrid, 0);
        Grid::SetColumnSpan(g_buttonGrid, std::max(1, (int)gridParent.ColumnDefinitions().Size()));
        Canvas::SetZIndex(g_buttonGrid, 1000);
        g_buttonGrid.IsHitTestVisible(true);
    } else if (liveColumn >= 0) {
        Grid::SetColumn(g_buttonGrid, liveColumn);
        g_injectedColumn = liveColumn;
        Canvas::SetZIndex(g_buttonGrid, 10000);
    }
    gridParent.Children().InsertAt(idx, g_buttonGrid);
    if (g_startOverlayMode)
        PositionButtonGridNearStart();
}

// ============================================================
// Apply / cleanup
// ============================================================

static void ApplyAllSettings() {
    HWND hWnd = FindCurrentProcessTaskbarWnd();
    if (!hWnd) { Wh_Log(L"[Apply] No taskbar window"); return; }
    g_taskbarWnd = hWnd;

    try {
        auto xamlRoot = GetTaskbarXamlRoot(hWnd);
        if (!xamlRoot) { Wh_Log(L"[Apply] GetTaskbarXamlRoot failed"); return; }
        auto root = xamlRoot.Content().try_as<FrameworkElement>();
        if (!root) { Wh_Log(L"[Apply] No XAML root content"); return; }

        if (InjectButtonGrid(root))
            StartNotificationThread();
        else
            Wh_Log(L"[Apply] Injection failed");
    } catch (...) {
        Wh_Log(L"[Apply] Exception during injection (XAML not ready)");
    }
}

static void ApplyAllSettingsOnWindowThread() {
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;
    RunFromWindowThread(hWnd, [](void*) { ApplyAllSettings(); }, nullptr);
}

// ============================================================
// Hooks
// ============================================================

using IconView_IconView_t = void* (WINAPI*)(void* pThis);
IconView_IconView_t IconView_IconView_Original;

void* WINAPI IconView_IconView_Hook(void* pThis) {
    auto result = IconView_IconView_Original(pThis);
    try {
        if (g_unloading) return result;
        // Once the primary grid exists this hook is only needed to discover
        // secondary taskbars, which only matters with the multi-monitor toggle on.
        if (g_buttonGrid && !g_settings.allTaskbars) return result;

        // Defer until the element is live in the XAML tree. Calling ApplyAllSettings
        // immediately from the constructor fires before the XamlRoot is stable, causing
        // null dereferences and WinRT exceptions that propagate through WH_CALLWNDPROC
        // and crash the process on startup.
        FrameworkElement iconView = nullptr;
        ((IUnknown**)pThis)[1]->QueryInterface(winrt::guid_of<FrameworkElement>(),
                                               winrt::put_abi(iconView));
        if (!iconView) {
            // Fallback: element isn't a FrameworkElement; try immediate path.
            ApplyAllSettingsOnWindowThread();
            return result;
        }

        g_autoRevokerList->emplace_back();
        auto autoRevokerIt = std::prev(g_autoRevokerList->end());
        *autoRevokerIt = iconView.Loaded(
            winrt::auto_revoke_t{},
            [autoRevokerIt](winrt::Windows::Foundation::IInspectable const& sender,
                            auto const&) {
                g_autoRevokerList->erase(autoRevokerIt);
                try {
                    if (g_unloading)
                        return;
                    if (!g_buttonGrid)
                        ApplyAllSettingsOnWindowThread();
                    // The sender's XamlRoot identifies the taskbar hosting this icon —
                    // register it if it is a secondary taskbar.
                    if (g_settings.allTaskbars) {
                        if (auto fe = sender.try_as<FrameworkElement>())
                            RegisterSecondaryTrayFromElement(fe);
                    }
                } catch (...) {
                    LogCurrentUiException(L"IconView Loaded");
                }
            });
    } catch (...) {
        LogCurrentUiException(L"IconView hook");
    }

    return result;
}

using LoadLibraryExW_t = HMODULE (WINAPI*)(LPCWSTR, HANDLE, DWORD);
LoadLibraryExW_t LoadLibraryExW_Original;

HMODULE WINAPI LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    HMODULE hModule = LoadLibraryExW_Original(lpLibFileName, hFile, dwFlags);
    if (hModule && lpLibFileName)
        HandleLoadedModuleIfSystemTray(hModule, lpLibFileName);
    return hModule;
}

// ============================================================
// Symbol hook setup
// ============================================================

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
    };
    return WindhawkUtils::HookSymbols(h, taskbarDllHooks, ARRAYSIZE(taskbarDllHooks));
}

static bool HookSystemTraySymbols(HMODULE hModule) {
    // SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll
    WindhawkUtils::SYMBOL_HOOK systemTrayHooks[] = {{
        {LR"(public: __cdecl winrt::SystemTray::implementation::IconView::IconView(void))"},
        &IconView_IconView_Original, IconView_IconView_Hook,
    }};
    if (!WindhawkUtils::HookSymbols(hModule, systemTrayHooks, ARRAYSIZE(systemTrayHooks))) {
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

// ============================================================
// Windhawk lifecycle
// ============================================================

BOOL Wh_ModInit() {
    Wh_Log(L"[Init] VD Switcher v2.0");
    LoadSettings();
    DetectExplorerBuild();

    if (!HookTaskbarDllSymbols())
        Wh_Log(L"[Init] taskbar.dll hooks failed — GetTaskbarXamlRoot unavailable");

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
            WindhawkUtils::SetFunctionHook(pLoadLibraryExW,
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

    g_retryStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    g_retryThread = CreateThread(nullptr, 0, [](void*) -> DWORD {
        for (int i = 0; i < 5 && !g_unloading; i++) {
            if (WaitForSingleObject(g_retryStopEvent, 2000) != WAIT_TIMEOUT) break;
            if (g_buttonGrid || g_unloading) break;
            Wh_Log(L"[AfterInit] Retry %d", i + 1);
            ApplyAllSettingsOnWindowThread();
        }
        return 0;
    }, nullptr, 0, nullptr);
}

void Wh_ModUninit() {
    g_unloading = true;
    Wh_Log(L"[Uninit]");

    // Drain any in-flight SwitchToDesktop background threads before stopping
    // the notification thread. Those threads access COM and mod globals; if they
    // outlive the DLL they crash. The click handler already guards with g_unloading,
    // so threads entering after this point exit immediately.
    while (g_activeSwitchThreads.load() > 0) Sleep(20);

    StopRetryThread();
    StopNotificationThread();

    // RunFromWindowThread is synchronous — blocks until the UI thread has removed the grid,
    // so all WinRT object lifetimes are safe and no FreeLibrary dance is needed.
    // Clear pending Loaded revokers on the UI thread so WinRT auto-revoke objects
    // are destroyed on the correct thread before the DLL is unloaded.
    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (hWnd) {
        RunFromWindowThread(hWnd, [](void* parameter) {
            HWND hWnd = static_cast<HWND>(parameter);
            if (!GetTaskbarXamlRoot(hWnd)) {
                Wh_Log(L"[Uninit] No live XAML root; retaining XAML state");
                return;
            }
            // Controlled UI-thread unload: revoke/release on this thread, then
            // reset() the no_destroy optionals so their heap buffers are freed
            // (a bare no_destroy container would keep its capacity forever).
            g_autoRevokerList->clear();
            RemoveButtonGrid();
            RemoveSecondaryBars();
            ClearAllButtonEventState();
            g_autoRevokerList.reset();
            g_buttonEventStates.reset();
            g_secondaryBars.reset();  // release WinRT refs on the UI thread
        }, hWnd);
    } else {
        // Explorer shutdown doesn't guarantee a usable XAML/UI thread. The
        // no_destroy owners deliberately retain state rather than releasing it
        // from Windhawk's unload thread after framework teardown.
        Wh_Log(L"[Uninit] No taskbar UI thread; retaining XAML state");
    }
}

void Wh_ModSettingsChanged() {
    LoadSettings();
    Wh_Log(L"[Settings] Changed");

    StopRetryThread();
    // Stop desktop-change callbacks before rebuilding tray columns. A late
    // callback during settings save can otherwise rebuild the old bar while the
    // UI thread is removing/reinserting columns.
    StopNotificationThread();

    HWND hWnd = g_taskbarWnd ? g_taskbarWnd : FindCurrentProcessTaskbarWnd();
    if (!hWnd) return;

    RunFromWindowThread(hWnd, [](void* parameter) {
        HWND hWnd = static_cast<HWND>(parameter);
        if (!GetTaskbarXamlRoot(hWnd)) {
            Wh_Log(L"[Settings] No live XAML root; deferring reapply");
            return;
        }
        RemoveButtonGrid();
        ApplyAllSettings();
        // Apply the new settings (including the multi-monitor toggle) to any
        // secondary taskbars discovered earlier.
        RefreshSecondaryBars();
    }, hWnd);
}
