# Taskbar Virtual Desktop Switcher

A [Windhawk](https://windhawk.net) mod for Windows 11 that injects clickable buttons into the system tray — one per virtual desktop — for instant switching without opening Task View.

![Three desktops with lower master button](assets/simple3wlowmaster.png)
*Three desktops with the optional Task View button as a lower sliver.*

![Default tray placement — three numbered buttons, first active](assets/simple3.png)
*Default tray placement: three desktops in a row, desktop 1 active.*

![Two desktop compact tray placement](assets/simple2.png)
*Compact tray placement with two desktops.*

![Four desktops with master button](assets/simple4wmaster.png)
*Four desktops with the optional Task View button.*

![Taller taskbar with right-side grid and lower master button](assets/gridonrightwlowermaster.png)
*Taller taskbar: a dense grid with the Task View button as a lower sliver.*

![Eleven desktops in a grid on a busy double-height taskbar](assets/busy-many-desktops-modified-taskview.png)
*Eleven desktops on a double-height taskbar, with a restyled Task View button
and several other taskbar mods alongside. `auto` fits the grid to the height it
is given rather than to a desktop count.*

![Left of Start button](assets/left-of-start.png)
*Start placement: switcher reserved to the left of Start.*

![Left of Start with Start hidden](assets/left-of-start-hidden-start.png)
*Start placement with the Start button hidden.*

![Over Start, nudged above](assets/over-above-start.png)
*Overlay mode can be nudged up with the vertical offset setting.*

![Over Start, nudged below](assets/over-below-start.png)
*Overlay mode can also be nudged down.*

![Right of Start with Start hidden](assets/right-of-start-hidden-start.png)
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

**[Vertical OmniButton archive](../omnibutton-customizer/archive/vertical-omnibutton-v1.4.wh.cpp)** (this lab, by sb4ssman) — source of the `GetTaskbarXamlRoot` boilerplate, `RunFromWindowThread` dispatcher, `FindCurrentProcessTaskbarWnd`, and the `IconView::IconView` hook-and-retry injection pattern.

**[windows-11-taskbar-styler](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/windows-11-taskbar-styler.wh.cpp)** — reference for the `SystemTrayFrameGrid` XAML tree structure and element names (`ShowDesktopStack`, `NotificationCenterButton`, `ControlCenterButton`, `NotifyIconStack`).

**[Windhawk](https://windhawk.net)** by [m417z](https://github.com/m417z) — the modding platform that makes all of this possible.
