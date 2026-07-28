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
| Surface / State | `button-surface.h` (surfaces the mod owns) |
| Surface | `native-glyph-surface.h` (native items the mod borrows) |
| (all) | `taskbar-xaml-lifecycle.template.cpp` |

`button-surface.h` and `native-glyph-surface.h` are not interchangeable. The
first styles a XAML `Button` the mod created and fully controls. The second
styles something Windows drew, which the mod must probe before it can style —
see "Offer only the controls the item can honor" under Surface.

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
  - NewItems: append
    $name: Newly created items
    $description: >-
      What happens to an item your arrangement does not name. Only applies to
      a written arrangement - "auto" always includes everything.
    $options:
    - append: Add them after the arrangement
    - ignore: Leave them out until I add them
  $name: Layout
```

`NewItems` is required for any mod whose item set is dynamic, and omitted for a
fixed set. Without it, creating a desktop or a folder that the user's written
arrangement does not name makes that item silently unreachable — it resolves to
nothing and disappears. `MissingTokens` + `AppendMissing` implement the append
policy; log when items were appended so the user knows to fold them in.

**Sizing an item against its group.** An item that should match its neighbours
— a Task View button that is a full-height column beside the buttons, or a
full-width sliver above them — cannot be given a fixed width and height,
because in a hand-written arrangement the mod does not know which way it will
land. Return `AlongAxis(thickness, span)` from the size resolver instead:
`thickness` runs along the group's axis and `span` across it, with `span = 0`
meaning "match the rest of the group". One pair of settings then covers both
orientations wherever the user puts the item.

**The `auto` shape rule.** Deterministic, not scored. Compute `maxRows` from the
taskbar height **in DIPs** (never raw `GetWindowRect` pixels — see the DPI note
below) divided by the item pitch. Then take the **smallest column count
reachable within those rows** — that is what "use the available height" means —
and among the row counts producing it, the one with the **fewest empty slots**.
Four items with three rows available gives 2x2, not a ragged 3+1; five items
with four rows available gives 3x2, not 4+1. `FillOrder` decides whether that
shape fills across or down; `Justify` aligns the ragged tail.

**Offsets live in the expression**, not in their own settings. One string,
nothing to keep in sync with anything else. `1[+2,-1]` moves one item;
`(1, 2)[3,0]` moves a whole parenthesized group. Both are cosmetic — neither
changes the measured size or moves a neighbor. A separator is always required:
`1 (2 | 3)` is a parse error, not an implicit `1 | (2 | 3)`, so a missing
separator surfaces in the log instead of silently becoming another layout.

**Token vocabulary.** A token is an item's stable identity, never its displayed
label — labels are not unique, can contain the expression's own delimiters, can
be empty or an emoji, and renaming one would silently break an arrangement the
user wrote. Each mod declares and documents its vocabulary:

- fixed set → semantic names: `wifi`, `volume`, `battery`, `percent`, `clock`
- dynamic set → `1`, `2`, `3`, … because the set changes at runtime
- either → any extra named item, e.g. `master` for a Task View button

A dynamic mod may accept a readable alias for a number (`desktop2` ≡ `2`).
Matching is case-insensitive. Log the token-to-label map next to the
arrangement (`tokens: 1=Home  2=Work  master=Task View`) so a user can tell
which number is which item without the arrangement depending on the labels.

For a mod that today has an `itemOrder` string (OmniButton, Tray Utility),
`Layout.Arrangement` **replaces** it: an expression already encodes order and
grouping, so keeping both would be two strings describing one thing.

**DPI.** Taskbar height comes from `GetWindowRect` in physical pixels while all
XAML sizes are DIPs. Convert before dividing:
`MulDiv(r.bottom - r.top, 96, GetDpiForWindow(hWnd))`. Mixing the two is the
blocking bug flagged on PR #4855 and #4843.

**Reserve before you divide.** `maxRows` is the height available to the ITEM
GRID, not to the whole taskbar. Subtract everything else that occupies vertical
space first — `Adjust.PadY` on both sides, and any extra item shaped as a row
(a sliver above or below the grid). Skipping this lets the grid claim height
that is already spoken for, and the assembled group overflows the taskbar: four
22px items with a 6px sliver on a 96 DIP taskbar measures 102. A purely
cosmetic offset is NOT reserved — letting a sliver hang past the edge is
usually the point of it.

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

**These settings describe a GLYPH, not TEXT.** `ItemWidth` is the box a
character is centered in. It cannot also describe a string: "9%", "80%", and
"100%" are three different widths, a font or locale change moves them again,
and a battery percentage grows while the user watches it. Give a text item the
same fixed width as its neighbours and you reserve too little space — the
overflow then surfaces at paint time, as a clipped edge.

Measure it instead. `SizeResolver` is a callback precisely so a mod can answer
with something it measured: `native_glyph_surface::MeasureNatural` on the live
element, fed through `ngl::ContentAlong(measured, minimum, cross)`, so the
arrangement RESERVES the real width and the group's total grows to match. Keep
`ItemWidth` as the minimum so a short value still lines up with the glyphs
above it, round the measurement up, and add a pixel or two of slack or the item
re-measures every time its text ticks over. Re-check cheaply (`ActualWidth` is
a free property read) and re-apply if the value later outgrows its cell.

This was the OmniButton's clipped battery percentage, found 2026-07-25.

**`ItemWidth: 0` means "fit each item to its own content", and it is usually
the right default.** A fixed item width is a BOX, and the difference between
the box and the glyph inside it is dead space the arrangement itself
contributes. A native tray glyph is about 16px wide; at `ItemWidth: 32` that is
16px of nothing per item. Stacked two-by-two it reads as generous spacing and
nobody notices. Strung out in a single row it is half the control, and the user
correctly reports that the button is enormous around its own icons.

There is no padding setting that can fix it, and users will reach for one —
outer padding is outside the group by definition and can never change the gap
between two items. Offer the fit instead, and say so in the `$description` of
all three settings so the wrong knob names the right one.

Mechanically it is the same content-sizing path as the text case above, minus
the slack: a glyph does not grow, so padding the measurement only puts the dead
space back. Measure sticky-widest-seen — the first measure can land before the
item's XAML template has expanded and honestly report 0 — fall back to a
constant so a group is never arranged at zero width, and ask for ONE bounded
re-arrange when a fallback was used. Measure *after* applying the mod's own
glyph-size and font settings, or the cells are reserved for the native size and
then painted at a different one.

**`ItemSpacing` should go negative.** It is the only setting that can pull items
closer than touching, and the arranger handles a negative gap natively. Clamping
it at 0 leaves a user who wants a denser cluster with nothing to turn.

Both found in the OmniButton, 2026-07-26, from the same report.

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

**A mod that ZEROES its host's native padding MUST supply its own.** The
canonical default above is 0, and that is right for a group the mod created in
space it owns. It is WRONG for a mod that has taken over a native control's
content area: hosts like the OmniButton have ROUNDED CORNERS, and an item
arranged flush against that edge has its last pixels shaved by the curve.
Nothing overflows — the arithmetic is exact and every overflow check correctly
stays quiet — the content simply has nowhere to breathe.

OmniButton is the worked example, measured 2026-07-26: content ended at 66.6
in a button 67.0 wide, and the "%" of the battery percentage was clipped by
0.4px of margin. Its `PadX` therefore defaults to 4, and the setting
description says why. **Do not "correct" such a default back to 0 on the
grounds that the contract says 0** — that is precisely the mistake that
reintroduced this bug. If a mod deviates from a canonical default, the reason
belongs in the setting's own `$description`, where the next reader will find it
before changing it.

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

**Offer only the controls the item can honor.** A mod styling a NATIVE element
does not get to assume what it is. A Windows 11 tray icon is usually a font
glyph in a `TextBlock` named `InnerTextBlock`, where color, size, and font
family all apply — but the OmniButton's battery is drawn from *shapes*, which
is how it shows a fill level and a charging bolt at all. There is no font there
to size or replace.

A blind "first `TextBlock` anywhere below" search still finds *a* `TextBlock`
under a drawn icon and binds to it, so the settings look wired up and silently
do nothing. Probe with `native_glyph_surface::Probe`, read
`Surface::Supports()`, and omit the settings that come back false — the mod's
settings block should not advertise a font-family box for something with no
font. Log the probe result so a build that changes the tree is visible in the
log rather than as a setting that quietly stopped working.

Precedence is: the host itself is a `TextBlock` → a descendant named
`InnerTextBlock` → any `Shape` descendants → any `TextBlock` at all, flagged as
a guess. Shapes must outrank an unnamed `TextBlock`; the reverse order is the
bug this replaces.

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

## Taskbar position, and living with the rest of the ecosystem

Windows 11 only puts the taskbar at the bottom. Two mods by m417z move it, and
both are part of the ecosystem these mods must coexist with. Every mod in this
family checks `taskbar_host::LayoutModelApplies` BEFORE touching anything.

- **[taskbar-on-top](https://windhawk.net/mods/taskbar-on-top) — supported.**
  Nothing here positions against screen coordinates; everything is relative to
  the taskbar's own XAML tree, so a top taskbar is the same tree at a different
  y. Test it, don't special-case it.
- **[taskbar-vertical](https://windhawk.net/mods/taskbar-vertical) — NOT
  compatible, by construction.** It walks the identical
  `ControlCenterButton > Grid > ContentPresenter > ItemsPresenter > StackPanel`
  path and owns `RenderTransform` on those children to rotate them. This
  family's positioning writes `RenderTransform` on the same elements. One
  dependency property, two owners, last writer wins — cooperation cannot fix
  it. m417z documents the same class of conflict for `taskbar-multirow`.

**The rule: detect the condition, stand down completely, and say so in the
log.** Detect via the taskbar's own rect aspect (`taskbar_host::GetMetrics`),
never by sniffing for a specific mod — the aspect is the thing that actually
matters and stays true however the taskbar got that way. Standing down means
leaving the taskbar EXACTLY as found, not a half-applied layout. Report it in
both READMEs the way m417z does: name the mod, say why, say what happens.

A mod that arranges into a coordinate space someone else is rotating produces
garbage the user cannot diagnose, and it will be reported as *our* bug.

## Settings that drive a WINDOWS setting

A setting that reaches out and changes shared machine state — a registry value
Settings, Explorer, and other apps also read — is a different animal from one
that changes something the mod owns, and it goes through
`os-setting-bridge.h`. Three rules, and breaking any of them yields a toggle
that looks connected and is not:

- **Write the value Windows actually reads.** These names are a graveyard of
  near-misses. The taskbar battery percentage is `IsBatteryPercentageEnabled`;
  `TaskbarBatteryPercent` sits in the same key, reads exactly as plausible, and
  Windows 11's Settings app ignores it. OmniButton wrote the wrong one for
  months while its own log reported success on every write. **Verify against
  the live registry with the OS UI open** — toggle it there and watch which
  value moves. Never infer a registry name, and never trust a log line that
  only proves you wrote something somewhere.
- **Write every confirmed alias, restore every one.** Builds disagree about
  which is authoritative and a mod cannot cheaply detect the build's opinion.
- **Restore exactly.** A value the mod created where none existed is DELETED on
  unload, not set to zero. Leaving a zero behind is a mod permanently editing
  the user's machine.

Do not re-assert the value on a timer. If the user changes it in Settings, they
win; re-assert only when the mod's own settings change. Say plainly in both
READMEs that the toggle drives the Windows setting, and name the Settings page
it corresponds to.

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
