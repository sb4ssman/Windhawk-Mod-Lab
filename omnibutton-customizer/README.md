# OmniButton Customizer

A [Windhawk](https://windhawk.net) mod for Windows 11 that takes the native
OmniButton — the network / volume / battery cluster that opens Quick Settings —
and lets you arrange its items into any shape you like, hide the ones you don't
want, and restyle each one independently. It works on a taskbar at any edge of
the screen.

These native icons are compound and a little weird — each is several glyphs
layered on top of one another, each with its own built-in visual origin — so a
mathematically correct grid does not necessarily look like one. `auto` gets the
shape right, but it does not make pleasing arrangements; expect to adjust.
Where an example below shows its arrangement, that is the real string, nudges
and all.

![All four items as a 2×2 block on a single-height taskbar](assets/single-height-auto.png)
*Straight out of the box: `Arrangement` left at `auto`, which fits the four
native items to the taskbar height and settles on a 2×2 block.*

## Showcase

![The same 2×2 block, written by hand](assets/single-height-arranged-2x2.png)
*The same shape written out and optically corrected. The parentheses matter,
because `,` binds tighter than `|`:*

```text
(network[-6,2] | volume[-2,4]), (battery[1,0] | percent[4,-2])
```

![The four items arranged as a diamond, with the native tooltip showing](assets/single-height-diamond-adjusted-with-hover.png)
*A diamond — volume on top, battery below, network and the percentage on the
sides. No parentheses needed this time. It is still the native button, so the
tooltip and the click through to Quick Settings work as they always did:*

```text
network[4,-2] | volume[2,-4], battery[0,4] | percent[0,-2]
```

![Network and volume above a centered battery](assets/single-height-arranged-2-over-1.png)
*Three items with the percentage left out: two across the top, the battery
centered below. `Short row or column` decides how a ragged last group lines up.*

![The items in a single row](assets/single-height-arranged-reverse.png)
*A single row, in an order you choose rather than the native one.*

![A tight two-high stack](assets/single-height-compact-stack.png)
*Pulled in close with a negative `Size` → `Item spacing` — the setting that
tightens a cluster.*

![A tight cluster with an enlarged battery percentage](assets/single-height-stacked-with-percent-emphasis.png)
*The percentage enlarged with `Surface` → `Battery percentage size`.*

![A recolored battery percentage in a busy tray](assets/with-colors.png)
*Per-item color — here the battery percentage. An empty color leaves an item
exactly as Windows drew it.*

![All four items arranged vertically on a double-height taskbar](assets/double-height-arranged-vertical.png)
*A single column on a double-height taskbar, alongside several other tray and
taskbar mods in a dense two-row tray:*

```text
network[-2,6], volume[0,2], battery[0,0], percent[2,-6]
```

![A 2x2 block on a taskbar at the top of the screen](assets/top-2x2.png)
*A taskbar at the top: arrangements work exactly as they do at the bottom.*

![All four items in one evenly spaced row on a left taskbar](assets/side-left-single-row.png)
*A taskbar on the left, with the four items nudged onto evenly spaced points
across the button:*

```text
percent[-12,-1] | battery[-3,0] | volume[8,0] | wifi[14,0]
```

![A three-row arrangement on a left taskbar](assets/side-left-stacked.png)
*Rows stack on a side taskbar too. Windows gives the button a fixed height
there; the mod grows it to fit.*

![Battery over its percentage between network and volume on a right taskbar](assets/side-right-diamond.png)
*A taskbar on the right: the battery over its percentage, between network and
volume, with `Adjust` → `Horizontal padding` at 8:*

```text
wifi[-16,0] | (battery[-1,2], percent) | volume[16,0]
```

## Features

- Arrange network, volume, battery, and the battery percentage into any grid —
  automatically fitted to your taskbar height, or written out by hand
- Turn any of the four items off individually
- Writes no Windows settings and no registry values — it arranges the taskbar
  and nothing else
- Independent color and opacity per item, plus size and font family on the
  battery percentage, the one item that is really a single piece of text
- Per-item and per-group pixel nudges inside the arrangement expression
- Group padding and offset for positioning the cluster inside the button
- Keeps the native button in its native tray position, so other mods' "before
  OmniButton" anchors still mean what they always did
- No XAML Diagnostics, so it coexists with Windows 11 Taskbar Styler

If you installed an unpublished 1.x build from the pull request, re-apply your
customizations once: 2.x uses grouped settings and the Arrangement field, and
Windhawk cannot migrate renamed setting keys.

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
  network, battery | volume, percent     a 2x2 block
  network | volume | battery | percent   a single row
  network, volume, battery, percent      a single column
  network | volume | (battery, percent)  battery stacked over its percentage
  network | (volume, battery) | percent  a diamond
  ```

  Order of operations: parentheses first, then `,`, then `|` — so
  `a | b, c | d` is three columns with `b` stacked over `c`, and a 2×2 block
  is `(a | b), (c | d)` or `a, c | b, d`.

  The tokens are `network` (or `wifi`), `volume`, `battery`, and `percent`,
  and they are case-insensitive. A name the mod does not know is ignored and
  logged. `network` is the one native slot whose glyph changes
  between Wi-Fi, Ethernet, disconnected, airplane-mode, and VPN states. A
  separator is always required — `network (volume | battery)` is an error,
  not a shorthand for `network | (volume | battery)`.

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
network[+2,-1] | volume | battery   network moves 2px right and 1px up
(battery, percent)[3,0] | network   the stacked pair moves 3px right
```

Each expression nudge is clamped to ±100 pixels on each axis; the whole-group
Adjust offsets are clamped to ±40. Offsets are cosmetic. Nothing else shifts, and the group's overall size does
not change. To move the whole cluster instead, use `Adjust` → horizontal and
vertical offset. These replace the eight per-item nudge settings that 1.x had.
Nudges and offsets are screen pixels on every taskbar edge — see
[Other taskbar positions](#other-taskbar-positions).

**The percentage arriving late.** An arrangement you write names the items that
existed when you wrote it. Turn the battery percentage on afterwards and it is
in no group, so by default it is appended after your arrangement rather than
vanishing — the log says when that happened, so you can fold it in when you
next edit. Set `Layout` → `Items your arrangement does not name` to *Leave it
out* if you would rather your arrangement be the whole truth. `auto` always
includes every enabled item.

## The battery percentage

**Whether the percentage exists is Windows' decision, not this mod's.** Turn it
on or off in **Settings → System → Power & battery → Battery percentage**. This
mod does not write that setting, or any other Windows setting.

`Content` → `Battery percentage` hides the percentage from the arrangement,
exactly like the three toggles above it hide their own items. All four mean the
same thing, and none of them reaches outside the taskbar. If Windows isn't
showing the percentage, there is nothing here to arrange or hide and the toggle
does nothing.

*An earlier version did drive the Windows setting. It was removed: even with
the correct registry value and a change broadcast, Explorer only sometimes
re-read it and the Settings page never refreshed, so the control worked once
and then appeared dead. A switch that behaves that way is worse than no switch.*

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

### Content

| Setting | Default | Description |
|---------|---------|-------------|
| Network | On | Wi-Fi, Ethernet, disconnected, airplane-mode, and VPN states share this one native slot |
| Volume | On | |
| Battery | On | Absent on machines without a battery |
| Battery percentage | On | Hides it from the arrangement. Windows decides whether it exists — Settings → System → Power & battery |

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
| Network / Volume / Battery / Battery percentage color | *(native)* | Empty preserves the native color |
| Network / Volume / Battery / Battery percentage opacity | -1 | -1 is the native opacity; otherwise 0–100% |
| Battery percentage size | 0 pt | 0 is the native size; clamped to 0–64 |
| Battery percentage font family | *(native)* | Empty preserves the native font |

**Why only the percentage has a size and a font.** Because only the percentage
is a single piece of text. Each of the other three is a **stack of glyphs
layered exactly on top of one another** — network and volume are three deep
(Windows calls them Underlay, Base, and AccentOverlay), the battery is two, an
outline and a fill. That stacking is how one icon shows signal strength, a mute
slash, or a charge level.

Resize or re-font one layer of a stack and it stops coinciding with the others:
you get a larger glyph ghosting over the original rather than a bigger icon. So
those two controls are not offered for items that are stacked, and the mod
works out which is which by counting the glyphs rather than assuming.

Color and opacity work on all four. Color is applied where every layer inherits
it, so a stack recolors as one — except the battery, whose layers have no
shared parent to write to; there the outline recolors reliably and the fill only
on some Windows builds. The log says what it found. Opacity applies to the whole
item rather than to any glyph inside it, so it is always safe.

All color settings accept `#RRGGBB` or `#AARRGGBB` hex (the alpha byte is
honored), the generics `accent`, `accentLight`, and `accentDark` for the
Windows accent shades, or `transparent` for a fully transparent glyph — nothing
drawn, the item still present and clickable. Leaving a color empty keeps the
native color.

## Other taskbar mods

**[Tray system icon tweaks](https://windhawk.net/mods/taskbar-tray-system-icon-tweaks)**
has its own `hideNetworkIcon` / `hideVolumeIcon` / `hideBatteryIcon` switches,
which cover the same three icons as this mod's `Content` switches. They are not
aware of each other: if an icon is hidden there and enabled here, this mod
reserves an arrangement cell for something Windows is not drawing. Pick one
mod to own the show/hide decision.

**[Multiple taskbars](https://windhawk.net/mods/taskbar-multi-tray).** This mod
resolves the primary `Shell_TrayWnd` only, so OmniButtons on secondary-monitor
taskbars keep their native arrangement. The primary one is arranged normally.

The mod deliberately does not move the native `ControlCenterButton` across tray
columns. Keeping it where Windows put it is what lets other mods' semantic
anchors — "before OmniButton", "before clock" — keep their established meaning.
Moving it would need a shared placement lease so two mods couldn't claim
contradictory anchor order, so this mod offers no placement setting at all.

## Other taskbar positions

Windows 11 can put the taskbar on any edge (Settings → Personalization →
Taskbar → Taskbar behaviors, on builds that have the setting). This mod reads
the edge Windows reports and follows a move while it is running. The last four
screenshots in the [Showcase](#showcase) are the top, left and right edges.

**Top — the same as bottom.** Everything is positioned relative to the
taskbar's own layout, never to screen coordinates, so a taskbar at the top
gets the same arrangement in a different place. This also covers
[Taskbar on top](https://windhawk.net/mods/taskbar-on-top).

**Left or right — your arrangement as written.** An arrangement you write is
laid out exactly as written on every edge: `|` side by side, `,` stacked, and
`[dx,dy]` always moves an item `dx` right and `dy` down. The horizontal and
vertical settings in `Adjust` stay horizontal and vertical, and nothing is
mirrored between left and right. Only `auto` changes: it fits the taskbar's
width instead of its height. Windows makes the OmniButton a fixed height on a
side taskbar, so the mod raises its minimum height to fit the arrangement.

**[Vertical Taskbar](https://windhawk.net/mods/taskbar-vertical) — supported
in its default native mode**, which uses the same native side taskbar. With
its "Use the native taskbar when possible" option turned off, it rotates a
horizontal taskbar instead, writing `RenderTransform` on the very OmniButton
elements this mod moves. One property cannot have two owners, so this mod
detects that case (the taskbar runs down a side while Windows still reports a
horizontal edge), leaves the native OmniButton completely untouched, and says
so in the Windhawk log.

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
- Turning the battery percentage on or off in Windows Settings sometimes needs
  the next Explorer start before the taskbar reflects it. That is Windows, not
  this mod — the arrangement follows whatever ends up on screen
- Switching every item off under `Content` leaves a small blank button rather
  than nothing at all. That button is still the way into Quick Settings, so it
  keeps a minimum clickable size on purpose
