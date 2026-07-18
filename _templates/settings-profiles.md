# Canonical Settings Profiles

Use the smallest set of profiles that truthfully describes a mod. Settings are
ordered by user intent, not by the order in which the code happens to read them.

## Loader rules

- Defaults live in the settings block. The second argument to
  `Wh_GetIntSetting` is a format argument, not a fallback value.
- `Wh_GetStringSetting` returns an allocated empty string for an unset string,
  not `nullptr`; test `value[0]` when emptiness matters.
- Do not declare a new canonical key with a default and then try to detect its
  absence with an integer sentinel. Windhawk has already materialized the
  declared default, so the legacy key will never be consulted.
- Published keys are persistent API. Prefer keeping a legacy key over a loader
  alias that cannot distinguish “unset” from “explicitly chose the default.”

## Canonical order

1. enablement and host placement
2. item/content list
3. group layout
4. item dimensions and spacing
5. content formatting
6. surface styling
7. state styling
8. group padding and offsets
9. per-item offsets
10. advanced behavior and diagnostics

## Host injection

Use for an independently injected tray group.

```yaml
- position: beforeIcons
  $name: Position
  $description: Where to place the group in the Windows 11 taskbar.
  $options:
  - beforeIcons: Before notification icons
  - beforeOmni: Before network, volume, and battery
  - beforeClock: Before clock
  - afterClock: After clock
  - afterShowDesktop: After Show Desktop
```

Only advertise anchors the implementation supports. Start-adjacent and overlay
positions belong to the StartPlacement extension, not the base profile.

## Group layout

Use for a repeated set of items. `availableRows` is calculated by code from the
taskbar height and item pitch; it is not a setting.

```yaml
- gridMode: autoSmart
  $name: Grid mode
  $options:
  - autoSmart: Smart automatic
  - singleRow: Single row
  - singleColumn: Single column
  - fixedRows: Fixed rows
  - fixedColumns: Fixed columns
  - fixedGrid: Fixed rows and columns

- smartLayout: balanced
  $name: Smart layout
  $options:
  - balanced: Balanced
  - packVertical: Pack vertical
  - packHorizontal: Pack horizontal

- gridRows: 0
  $name: Rows (0 = auto)

- gridColumns: 0
  $name: Columns (0 = auto)

- fillOrder: rowFirst
  $name: Fill order
  $options:
  - rowFirst: Row first
  - columnFirst: Column first

- shortGroupPosition: last
  $name: Short row or column
  $options:
  - first: First
  - last: Last

- shortGroupAlign: center
  $name: Short row or column alignment
  $options:
  - start: Start
  - center: Center
  - end: End
```

`shortGroupPosition` is optional. Omit it when only a trailing short group is
supported. Published VDS keys `buttonRows` and `buttonColumns` remain stable;
their UI labels can still use the canonical Rows and Columns language.

## Owned button surface

Use only when the mod creates or fully owns the `Button`.

The 5 canonical color slots are: text, background, hover background, pressed
background, border — in that order, followed by border thickness, corner
radius, opacity, shine. A mod may add an **optional identity axis** (e.g. VD
switcher's active/inactive desktop): the text and background slots each split
into per-identity settings, expanding in place in the same order. Interaction
states (hover/pressed) and border never split. Defaults must be generics or
empty — never a hardcoded hex color.

Color token convention (all color settings, all mods). Four generics:

- `accent` — the current Windows accent color.
- `accentLight` / `accentDark` — lighter/darker accent shades
  (AccentLight2 / AccentDark1). Natural hover/pressed values.
- `transparent` — fully transparent surface (alias for `#00000000`): nothing
  drawn, element still present and clickable.

Plus:

- `#RRGGBB` or `#AARRGGBB` — explicit color; the alpha byte is honored, so
  `#00000000` is equivalent to `transparent`.
- empty — the system/native default for that state. This must always mean
  "leave the native surface alone", including for active/highlight state
  colors: a mod that wants accent-by-default (e.g. VD switcher `activeColor`)
  ships `accent` as the literal default value rather than special-casing empty.
- Parsers also silently accept the numbered Windows shades `accentLight1`–`3`
  and `accentDark1`–`3`; these are intentionally left out of user-facing
  descriptions to keep them short.

```yaml
- buttonWidth: 32
  $name: Button width (px)

- buttonHeight: 22
  $name: Button height (px)

- buttonSpacing: 4
  $name: Button spacing (px)

- fontSize: 10
  $name: Font size (pt)

- textColor: ""
  $name: Text color
  $description: Hex (#RRGGBB or #AARRGGBB), the word accent, #00000000 for fully transparent, or empty for the system default.

- backgroundColor: ""
  $name: Background color
  $description: Hex (#RRGGBB or #AARRGGBB), the word accent, #00000000 for fully transparent, or empty for the system default.

- hoverBackgroundColor: ""
  $name: Hover background color
  $description: Hex (#RRGGBB or #AARRGGBB), the word accent, #00000000 for fully transparent, or empty for the native hover color.

- pressedBackgroundColor: ""
  $name: Pressed background color
  $description: Hex (#RRGGBB or #AARRGGBB), the word accent, #00000000 for fully transparent, or empty for the native pressed color.

- borderColor: ""
  $name: Border color
  $description: Hex (#RRGGBB or #AARRGGBB), the word accent, #00000000 for fully transparent, or empty for the system default.

- borderThickness: -1
  $name: Border thickness (px)
  $description: -1 preserves the native value; 0 removes the border.

- cornerRadius: -1
  $name: Corner radius (px)
  $description: -1 preserves the native value; 0 makes square corners.

- opacity: 100
  $name: Opacity (%)

- shineEffect: false
  $name: Shine effect
  $description: Adds a gradient highlight to custom color surfaces.
```

Active/inactive mods add state-specific colors; they do not replace the base
meaning of hover and pressed:

```yaml
- activeBackgroundColor: ""
- inactiveBackgroundColor: ""
- activeTextColor: ""
- inactiveTextColor: ""
- activeBold: false
```

## Group geometry

```yaml
- groupPaddingLeft: 0
  $name: Group padding left (px)
- groupPaddingRight: 0
  $name: Group padding right (px)
- groupPaddingTop: 0
  $name: Group padding top (px)
- groupPaddingBottom: 0
  $name: Group padding bottom (px)
- groupOffsetX: 0
  $name: Group horizontal offset (px)
- groupOffsetY: 0
  $name: Group vertical offset (px)
```

Padding participates in layout. Offset is a visual translation and should not
silently reserve space. Do not use one as an alias for the other.

## Per-item geometry

Use `<token>OffsetX` and `<token>OffsetY`, where token matches `itemOrder`.

```yaml
- wifiOffsetX: 0
- wifiOffsetY: 0
- volumeOffsetX: 0
- volumeOffsetY: 0
```

## Icon surface

Use for glyph/icon groups such as Privacy Anchor. Do not add button chrome.

```yaml
- iconSize: 16
- idleOpacity: 50
- activeOpacity: 100
- glowEnabled: false
- glowOpacity: 40
- slashColor: ""
- slashDirection: rising
- slashOpacity: 100
```

## Text panel

Use for Clock Spacer and future text panels.

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

Clock Spacer tracks an upstream settings language. Map these concepts in docs;
do not rename its published/upstream keys merely for symmetry.
