# OmniButton Customizer

A [Windhawk](https://windhawk.net) mod for Windows 11 that takes the native
OmniButton — the wifi / volume / battery cluster that opens Quick Settings —
and lets you arrange its items into any shape you like, hide the ones you don't
want, and restyle each one independently.

![Wifi and volume above battery and percentage](assets/compact-no-nudges.png)
*The four native items as a compact 2×2 block on a single-height taskbar.*

![Battery percentage, battery, volume, and wifi in one row](assets/re-odered-icons.png)
*The same items in a single row, reversed: percentage, battery, volume, wifi.*

![All four items on a double-height taskbar](assets/wifi-volume-batt-percent-vertical.png)
*A 2×2 block on a double-height taskbar, beside a wrapped clock.*

![Wifi, volume, and battery with the percentage off](assets/wifi-vol-batt-vertical.png)
*Three items with the battery percentage turned off.*

![A compact OmniButton in a busy multi-row tray](assets/in-a-busy-tray.png)
*Working alongside several other tray and taskbar mods in a dense two-row tray.*

## Features

- Arrange wifi, volume, battery, and the battery percentage into any grid —
  automatically fitted to your taskbar height, or written out by hand
- Turn any of the four items off individually
- The battery percentage toggle drives Windows' own setting, so it works
  whether or not you arrange anything
- Independent color and opacity per item, plus glyph size and font family
  wherever the item is actually text
- Per-item and per-group pixel nudges inside the arrangement expression
- Group padding and offset for positioning the cluster inside the button
- Keeps the native button in its native tray position, so other mods' "before
  OmniButton" anchors still mean what they always did
- No XAML Diagnostics, so it coexists with Windows 11 Taskbar Styler

## Upgrading from 1.x

Version 2.0 reorganizes every setting into groups — Placement, Content, Layout,
Size, Adjust, Surface — so this mod matches the rest of the family. Windhawk
cannot carry values across renamed keys, so **your previous customizations are
not migrated; re-apply them once after updating.**

`itemOrder` and the whole grid-mode family are gone, replaced by a single
**Arrangement** field. Grid mode, smart layout, fixed rows and columns, slot
width and height, the coupled/independent battery mode, and all eight per-item
nudge settings no longer exist; what replaced each of them is below.

Battery and percentage are now always two independent arrangement items. The
old coupled mode is not a mode any more — write them next to each other in the
arrangement and you have it, with the freedom to put them anywhere instead.

## The Arrangement field

`Layout` → `Arrangement` decides how the items are placed, and it is the only
field that does. Its default value is the word `auto`:

- **`auto`** fits the available items to the taskbar height. `Fill order`
  chooses whether they fill across rows or down columns; `Short row or column`
  aligns a ragged last group. The shape is worked out for you: the mod takes
  the narrowest grid that fits the height, preferring the one that wastes the
  fewest slots — four items on a standard taskbar become a 2×2 block, not a
  lopsided 3+1.
- **Anything else** is an arrangement you write. Names sit side by side with
  `|` and stack with `,`, and parentheses group them:

  ```text
  wifi, battery | volume, percent     a 2x2 block
  wifi | volume | battery | percent   a single row
  wifi, volume, battery, percent      a single column
  wifi | volume | (battery, percent)  battery stacked over its percentage
  ```

  The tokens are `wifi`, `volume`, `battery`, and `percent`, and they are
  case-insensitive. A separator is always required — `wifi (volume | battery)`
  is an error, not a shorthand for `wifi | (volume | battery)`.

**Omitting a token hides that item**, exactly like turning it off in `Content`.
Items Windows isn't showing at all — the battery on a desktop PC, for one — are
skipped silently whether you name them or not.

Every time the layout is applied, the arrangement `auto` produced is written to
the Windhawk log. Copy that line into the Arrangement field and you have the
automatic layout as a starting point to edit — the automatic and manual paths
are the same field and the same syntax. If what you write doesn't parse, the
log says what was expected and where, and the automatic arrangement is used
until you fix it.

**Nudging.** Append a pixel offset to any name to move just that item:

```text
wifi[+2,-1] | volume | battery      wifi moves 2px right and 1px up
(battery, percent)[3,0] | wifi      the stacked pair moves 3px right
```

Offsets are cosmetic. Nothing else shifts, and the group's overall size does
not change. To move the whole cluster instead, use `Adjust` → horizontal and
vertical offset. These replace the eight per-item nudge settings that 1.x had.

**The percentage arriving late.** An arrangement you write names the items that
existed when you wrote it. Turn the battery percentage on afterwards and it is
in no group, so by default it is appended after your arrangement rather than
vanishing — the log says when that happened, so you can fold it in when you
next edit. Set `Layout` → `Items your arrangement does not name` to *Leave it
out* if you would rather your arrangement be the whole truth. `auto` always
includes every enabled item.

## The battery percentage

`Content` → `Battery percentage` is not just a mod-side toggle: it drives the
same Windows setting as **Settings → System → Power & battery → Battery
percentage**. Turning the mod off restores whatever was there before the mod
first touched it, and a value the mod had to create is removed rather than left
behind as a zero. Windows sometimes only materializes the change on the next
Explorer start.

Because the percentage genuinely appears and disappears in the native tree,
the mod watches for it and re-applies the arrangement when it shows up or goes
away, rather than leaving a briefly unstyled percentage sitting in the cluster.

**It is text, so it does not use `Item width`.** "9%", "80%", and "100%" are
three different widths, and a font change moves them again. The percentage's
cell is measured from the text it actually contains — never narrower than
`Item width`, wider when it needs to be — so it cannot be clipped at the edge
of the group. If the value grows past the cell that was reserved for it, the
layout is re-applied at the new width. Every other item is a glyph and does
use `Item width`.

## Settings

### Placement

| Setting | Default | Description |
|---------|---------|-------------|
| Placement: not available in this mod | — | A note, not a control. The OmniButton stays in its native tray position; editing the box does nothing |

### Content

| Setting | Default | Description |
|---------|---------|-------------|
| Wifi | On | |
| Volume | On | |
| Battery | On | Absent on machines without a battery |
| Battery percentage | On | Also drives Windows' own battery-percentage setting |

### Layout

| Setting | Default | Description |
|---------|---------|-------------|
| Arrangement | `auto` | `auto`, or an arrangement you write — see above |
| Fill order | Fill rows first | Used by `auto` |
| Short row or column | Center | Used by `auto`; start, center, or end |
| Items your arrangement does not name | Add it after | Or leave it out; only applies to a written arrangement |

### Size

| Setting | Default | Description |
|---------|---------|-------------|
| Item width | 0 (fit) | 0 reserves exactly the width each item needs; a number puts every item in a fixed box of that width. Clamped to 0–80 |
| Item height | 24 px | Clamped to 16–80 |
| Item spacing | 0 px | Gap between items along each axis; negative pulls them together. Clamped to −16–40 |

**How to make the cluster tight.** Two settings change the space *between* the
icons, and horizontal padding is not one of them:

- **`Item width`** is the box each glyph is centered in. A native tray glyph is
  about 16px wide, so a 32px box is 16px of dead space per item — barely
  noticeable in a 2×2 block, and half the button in a single row. `0` sizes
  every item to its own content, which is why it is the default.
- **`Item spacing`** is the gap between those boxes, and it goes negative if you
  want them closer than touching.
- **`Horizontal padding`** is *outside* the whole group. It cannot change the
  distance between two items, and no value of it ever will.

If the button looks far bigger than the icons in it, `Item width` is almost
always the reason.

### Adjust

| Setting | Default | Description |
|---------|---------|-------------|
| Horizontal padding | 4 px | Reserved on both sides of the group; clamped to 0–24. **Not cosmetic — see below** |
| Vertical padding | 0 px | Reserved above and below the group; clamped to 0–24 |
| Horizontal offset | 0 px | Moves the group; reserves no space; clamped to ±40 |
| Vertical offset | 0 px | Moves the group up (negative) or down (positive); clamped to ±40 |

**Why horizontal padding defaults to 4 and not 0.** The mod zeroes the
OmniButton's own padding so the arrangement owns the entire content area — and
the button has rounded corners. An item arranged flush against that edge has
its last pixels shaved by the curve, which is exactly what used to clip the
"%" off the battery percentage. Those few pixels are the group's breathing
room, not decoration. Setting it to 0 is supported, but expect edge items to
touch the button's rounded border.

It is *outer* padding: it reserves space at the two ends of the group and can
never change the gap between two items. `Item width` and `Item spacing` do
that.

### Surface

| Setting | Default | Description |
|---------|---------|-------------|
| Wifi / Volume / Battery / Battery percentage color | *(native)* | Empty preserves the native color |
| Wifi / Volume / Battery percentage glyph size | 0 pt | 0 is the native size; clamped to 0–64 |
| Wifi / Volume / Battery percentage font family | *(native)* | Empty preserves the native font |
| Wifi / Volume / Battery / Battery percentage opacity | -1 | -1 is the native opacity; otherwise 0–100% |

**Why the battery has fewer controls.** Wifi, volume, and the percentage each
draw from a *single* glyph, so color, size, and font family all mean something.
The battery does not: it is two glyphs layered on top of each other — an
outline and a fill — which is how it can show a charge level and a charging
bolt at all. Resizing or re-fonting one of a stacked pair pulls the two apart,
so neither setting is offered. Its color is attempted anyway, on a best-effort
basis, and the mod logs what it found — depending on your Windows build it may
recolor only one of the two layers. Opacity works on all four, because it
applies to the item rather than to the glyph inside it.

All color settings accept `#RRGGBB` or `#AARRGGBB` hex (the alpha byte is
honored), the generics `accent`, `accentLight`, and `accentDark` for the
Windows accent shades, or `transparent` for a fully transparent glyph — nothing
drawn, the item still present and clickable. Leaving a color empty keeps the
native color.

## Other taskbar mods

The mod deliberately does not move the native `ControlCenterButton` across tray
columns. Keeping it where Windows put it is what lets other mods' semantic
anchors — "before OmniButton", "before clock" — keep their established meaning.
Moving it would need a shared placement lease so two mods couldn't claim
contradictory anchor order, which is why the Placement group is a note rather
than a control.

## Other taskbar positions

**[Taskbar on top](https://windhawk.net/mods/taskbar-on-top) — supported.**
Everything here is positioned relative to the taskbar's own layout, never to
screen coordinates, so a taskbar at the top is the same arrangement in a
different place.

**[Vertical Taskbar](https://windhawk.net/mods/taskbar-vertical) — not
compatible, and this mod stands down when it detects one.** Both mods reach the
same OmniButton elements and both position them by writing `RenderTransform`:
that mod rotates them, this one moves them into a grid. One property, two
owners — there is no arrangement in which both are correct. Rather than fight
over it and paint something broken, this mod detects a taskbar that runs down a
side, leaves the native OmniButton completely untouched, and says so in the
Windhawk log. Turn the vertical mod off and this one resumes on the next
Explorer restart.

## Taskbar Styler

Does not use XAML Diagnostics, so it is compatible with Windows 11 Taskbar
Styler. The mod leases the native elements' dependency properties and restores
each one's exact prior local value when it unloads.

## Known limitations

- The items may not appear arranged until the mod injects on the first tray
  icon load; the retry loop runs up to 5 times at 2-second intervals
- A glyph's color, size, and font wait for its XAML template to expand. That
  normally happens within a few layout passes; if an item's template never
  produces a text glyph, the log says so and the item keeps its native
  appearance
- The battery percentage may need the next Explorer start before Windows shows
  or hides it
