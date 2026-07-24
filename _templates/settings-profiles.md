# Settings Component Library

Every mod in this lab does the same four things: take or hijack a spot on the
taskbar, arrange items into a grid there, make those items interactive, and
style them. So they get the same settings, with the same keys, in the same
groups, in the same order.

This file is the contract. A mod **assembles** components; it never invents a
key, never renames one, and never reorders the groups. Omit a whole group when
the mod does not do that thing.

Each component pairs 1:1 with the code template that consumes it:

| Group | Code template |
|---|---|
| Placement | `injected-grid-column.h`, `start-placement.h` |
| Layout | `nested-group-layout.h` |
| Surface / State | `button-surface.h` |
| (all) | `taskbar-xaml-lifecycle.template.cpp` |

## Two platform facts this design is built on

**Nested groups are supported.** A group is a top-level key whose value is a
list of settings, with its own `$name` and optional `$description`. Sub-keys are
read as `Group.Key`:

```yaml
- Layout:
  - Arrangement: auto
    $name: Arrangement
  $name: Layout
```

```cpp
WindhawkUtils::StringSetting arrangement{Wh_GetStringSetting(L"Layout.Arrangement")};
```

Verified against `taskbar-elastic-pill` in the live catalog (`Animation.*`,
`Colors.*`), so the form is review-clean.

**The settings API is read-only.** `windhawk_api.h` exposes `Wh_GetIntSetting`
and `Wh_GetStringSetting` and no setter. A mod can never write a computed value
back into its own settings. Any design that depends on the mod filling a field
in for the user is impossible — see the `auto` sentinel in Layout for how this
is handled instead.

## Assembly order

Fixed. Omit groups; never reorder them.

| # | Group | What it answers |
|---|---|---|
| 1 | **Placement** | Where does the group go? |
| 2 | **Content** | Which items exist and what do they say? |
| 3 | **Layout** | How are they arranged? |
| 4 | **Size** | How big is each item? |
| 5 | **Adjust** | Where exactly does the whole group sit? |
| 6 | **Surface** | What does an item look like? |
| 7 | **State** | How does the current/active item differ? |
| 8 | **Behavior** | Everything else, experimental and diagnostic last. |

A group may append **mod-specific keys after its canonical keys**. It may not
insert them among the canonical keys, rename them, or reorder them. Example:
VD Switcher appends `TaskViewButton` / `TaskViewLabel` to Content and
`TaskViewWidth` / `TaskViewHeight` to Size.

## Loader rules

- Defaults live in the settings block. The second argument to
  `Wh_GetIntSetting` is a format argument, not a fallback value.
- `Wh_GetStringSetting` returns an allocated empty string for an unset string,
  not `nullptr`; test `value[0]` when emptiness matters.
- **Never write a legacy-key alias.** Windhawk materializes every declared
  default, so a loader can never tell "unset" from "explicitly chose the
  default", and the alias silently loses. This was tried in VD Switcher v1.6
  (`AliasedStr` / `AliasedInt`) and removed. Renaming a key is a clean break:
  bump the major version and say so in the README changelog.
- Grouping a mod's keys for the first time is exactly such a break. Do it once,
  at a major version, for the whole settings block — never half.

---

## 1. Placement

For an independently injected tray group.

```yaml
- Placement:
  - Position: beforeIcons
    $name: Position
    $description: Where to place the group in the Windows 11 taskbar.
    $options:
    - beforeIcons: Before notification icons
    - beforeOmni: Before network, volume, and battery
    - beforeClock: Before clock
    - afterClock: After clock
    - afterShowDesktop: After Show Desktop
  - AllTaskbars: false
    $name: Show on all taskbars
    $description: >-
      Experimental. Also injects into secondary monitors' taskbars. Secondary
      taskbars are discovered as their tray icons load, so an Explorer restart
      may be needed before the group appears.
  $name: Placement
```

Only advertise anchors the implementation supports. An unsupported combination
must fail and log; it must never silently fall back to column zero.

**StartPlacement extension.** Append these options only when the mod copies
`start-placement.h` and owns the entire group being placed:

```yaml
    - leftOfStart: Left of Start (experimental)
    - overStart: Over Start (experimental)
    - rightOfStart: Right of Start (experimental)
```

These reserve room in the centered taskbar-items area rather than a system-tray
column. Experimental because the internal Start/RootGrid layout changes between
Windows builds and can conflict with another mod that repositions Start.

## 2. Content

Which items exist and what each one displays. Contents are mod-specific; the
group name and its slot in the order are not. VD Switcher's instance:

```yaml
- Content:
  - LabelFormat: number
    $name: Label format
    $options:
    - number: "Numbers  1  2  3"
    - roman: "Roman numerals  I  II  III"
    - symbol: "Indicator symbols  ●  ○  ○"
    - custom: Custom labels
  - CustomLabels: ""
    $name: Custom labels
    $description: Comma-separated, e.g. "H,W,M". Used when Label format is Custom.
  - ActiveSymbol: "●"
    $name: Active indicator symbol
  - InactiveSymbol: "○"
    $name: Inactive indicator symbol
  $name: Content
```

## 3. Layout

One field decides the arrangement. Its default value is the word `auto`, which
means "let the mod pick the grid shape". Anything else is an explicit layout
expression. There is no mode toggle, because there is only one field.

```yaml
- Layout:
  - Arrangement: auto
    $name: Arrangement
    $description: >-
      "auto" fits the items to the available taskbar height. Anything else is
      an explicit layout: names side by side with "|", stacked with ",",
      grouped with parentheses — "1, 2 | 3, 4" is a 2x2 block. Append a pixel
      offset to any name to nudge just that item, e.g. "1[+2,-1]". Each time
      the layout is applied, the expression that "auto" produced is written to
      the Windhawk log, so you can paste it here and edit it.
  - FillOrder: rows
    $name: Fill order
    $description: Used by "auto". Whether items fill across rows or down columns first.
    $options:
    - rows: Fill rows first
    - columns: Fill columns first
  - Justify: center
    $name: Short row or column
    $description: Used by "auto". How a ragged last row or column is aligned.
    $options:
    - start: Start
    - center: Center
    - end: End
  $name: Layout
```

**The `auto` shape rule.** Deterministic, not scored. Compute `maxRows` from the
taskbar height **in DIPs** (never raw `GetWindowRect` pixels — see the DPI note
below) divided by the item pitch. Then take the **smallest column count
reachable within those rows** — that is what "use the available height" means —
and among the row counts producing it, the one with the **fewest empty slots**.
Four items with three rows available gives 2x2, not a ragged 3+1; five items
with four rows available gives 3x2, not 4+1. `FillOrder` decides whether that
shape fills across or down; `Justify` aligns the ragged tail.

**Per-item offsets live in the expression**, not in their own settings. One
string, nothing to keep in sync with anything else. Items are named by their
number (`1`, `2`, ...) plus any extra item the mod defines (`master`).

**DPI.** Taskbar height comes from `GetWindowRect` in physical pixels while all
XAML sizes are DIPs. Convert before dividing:
`MulDiv(r.bottom - r.top, 96, GetDpiForWindow(hWnd))`. Mixing the two is the
blocking bug flagged on PR #4855 and #4843.

## 4. Size

```yaml
- Size:
  - ItemWidth: 20
    $name: Item width (px)
  - ItemHeight: 22
    $name: Item height (px)
  - ItemSpacing: 2
    $name: Item spacing (px)
  $name: Size
```

Icon-only mods use `ItemSize` in place of width/height when the item is square.

## 5. Adjust

The one adjustment component. Two concepts, two axes, four numbers — nothing
gets four *sides*, ever. Items are centered by default; these exist purely to
buy back an effect the automatic centering doesn't give you.

```yaml
- Adjust:
  - PadX: 0
    $name: Horizontal padding (px)
    $description: Space reserved on both sides of the group. Participates in layout.
  - PadY: 0
    $name: Vertical padding (px)
  - OffsetX: 0
    $name: Horizontal offset (px)
    $description: Moves the group visually. Does not reserve space.
  - OffsetY: 0
    $name: Vertical offset (px)
  $name: Adjust
```

Padding participates in layout; offset is a visual translation. Never use one
as an alias for the other. The same X/Y idea applies to a single item through
the `name[dx,dy]` suffix in `Layout.Arrangement`.

## 6. Surface

Use when the mod creates or fully owns the item's surface. Canonical order:
text formatting, then the five color slots — text, background, hover
background, pressed background, border — then border thickness, corner radius,
opacity, shine.

```yaml
- Surface:
  - FontSize: 10
    $name: Font size (pt)
  - FontFamily: ""
    $name: Font family
    $description: Empty uses the native font.
  - TextColor: ""
    $name: Text color
  - BackgroundColor: ""
    $name: Background color
  - HoverBackgroundColor: ""
    $name: Hover background color
    $description: Empty brightens the item's own background; native surfaces keep the native hover.
  - PressedBackgroundColor: ""
    $name: Pressed background color
    $description: Empty darkens the item's own background; native surfaces keep the native pressed state.
  - BorderColor: ""
    $name: Border color
  - BorderThickness: 0
    $name: Border thickness (px)
  - CornerRadius: 4
    $name: Corner radius (px)
    $description: 0 is square; 4 is the Windows default.
  - Opacity: 100
    $name: Opacity (%)
  - ShineEffect: false
    $name: Shine effect
    $description: Adds a gradient highlight to custom color surfaces.
  $name: Surface
```

**Color token convention — every color setting in every mod:**

- `accent` — the current Windows accent color.
- `accentLight` / `accentDark` — lighter/darker accent shades (AccentLight2 /
  AccentDark1). Natural hover/pressed values.
- `transparent` — fully transparent surface (alias for `#00000000`): nothing
  drawn, element still present and clickable.
- `#RRGGBB` or `#AARRGGBB` — explicit color; the alpha byte is honored.
- empty — the native default for that state. This always means "leave the
  native surface alone", including for highlight colors: a mod that wants
  accent-by-default ships the literal string `accent` as the default rather
  than special-casing empty.
- Parsers also silently accept `accentLight1`–`3` and `accentDark1`–`3`. These
  stay out of user-facing descriptions to keep them short.

Defaults must be a generic token or empty — never a hardcoded hex color.

**Icon surface variant.** For glyph groups such as Privacy Anchor. Same slot,
no button chrome:

```yaml
- Surface:
  - IdleOpacity: 50
  - ActiveOpacity: 100
  - GlowEnabled: false
  - GlowOpacity: 40
  - SlashColor: ""
  - SlashDirection: rising
  - SlashOpacity: 100
  $name: Surface
```

## 7. State

Only for mods with an identity axis (VD Switcher's current desktop, a toggled
item). The text and background slots split per identity, keeping the canonical
order. Interaction states (hover, pressed) and border never split.

```yaml
- State:
  - ActiveTextColor: ""
    $name: Active text color
  - InactiveTextColor: ""
    $name: Inactive text color
  - ActiveBackgroundColor: accent
    $name: Active background color
  - InactiveBackgroundColor: ""
    $name: Inactive background color
  - ActiveBold: false
    $name: Bold the active item
  $name: State
```

When a mod has a State group, `Surface.TextColor` and
`Surface.BackgroundColor` are omitted — State replaces them rather than
competing with them.

## 8. Behavior

Mod-specific behavior, then experimental options, then diagnostics. Always
last.

```yaml
- Behavior:
  - HideWhenSingle: false
    $name: Hide when only one item
  $name: Behavior
```

## Text panel

Clock Spacer and future text panels track an upstream settings language rather
than this library. Map the concepts in docs; do not rename its published or
upstream keys for symmetry.

```yaml
- panelWidth: 0
- panelHeight: 0
- panelMaxWidth: 0
- lineSpacing: 0
- textAlignment: center
- fontSize: 0
- fontFamily: ""
- fontWeight: ""
- textColor: ""
- contentOffsetX: 0
- contentOffsetY: 0
```
