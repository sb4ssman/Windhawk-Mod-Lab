# Tray Utility Customizer

Granular, predictable control over the low-frequency Windows 11 system-tray
utility icons:

- **Show hidden icons** (the overflow chevron) — token `overflow`
- **Emoji and more** — token `emoji`
- **Touch keyboard** — token `touchKeyboard`
- **Pen menu** — token `penMenu`
- **Virtual touchpad** — token `virtualTouchpad`
- **Input/language indicator** — token `inputIndicator`

The native controls stay alive and Windows-owned — clicks, flyouts, and
tooltips are untouched. The mod gathers their hosts into one owned group and
positions each icon individually, at its native size by default.

![Native tray before the mod](assets/disabled.png)
*Mod disabled: the chevron, Emoji, and touch keyboard sit in their native positions.*

![One row at the hidden-icons position](assets/inline-overflow-emoji-touchkeyboard.png)
*The default `overflow | emoji | touchKeyboard`: one row, native sizes.*

![Chevron centered above a row of utilities](assets/overflow-over-utility-row.png)
*`overflow, (emoji | touchKeyboard)`: the chevron centered on its own row above the pair.*

![Chevron above the utility stack](assets/overflow-utility-stack.png)
*The chevron leading a stacked pair.*

![Utility stack above the chevron](assets/utility-stack-overflow.png)
*The same stack with the chevron last instead.*

![A neat column](assets/neat-stack.png)
*A full single column of utilities on a single-height taskbar.*

![A dedicated column elsewhere in the tray](assets/put%20it%20somewhere%20else.png)
*The group leased into its own tray column at the right end of the taskbar.*

![On a busy double-height taskbar](assets/busy-tray.png)
*Coexisting with a heavily modded double-height tray.*

![Beside Start](assets/unnecessary-but-possible.png)
*The experimental Right of Start position — unnecessary, but possible.*

## Layout expression

One string describes the whole arrangement:

- `|` places groups side by side along the **primary axis**
- `,` stacks items along the crossed axis
- parentheses nest, alternating axes
- every group is centered (or start/end-aligned) against its siblings

Examples with the primary axis set to Row:

- `overflow | emoji | touchKeyboard` — one row of three icons
- `overflow | emoji, touchKeyboard` — chevron centered beside a stacked pair
- `overflow | emoji, touchKeyboard | penMenu` — the diamond: two centered
  icons flanking a stacked middle column

Set the primary axis to Column and the same strings arrange top-to-bottom
instead. Icons omitted from the expression, and icons Windows currently
hides, simply don't participate; anything visible that you didn't place is
appended after the group so nothing is ever lost. The group re-adapts
automatically as icons appear and disappear (for example the transient touch
keyboard).

Tokens accept forgiving aliases: `chevron`/`hidden` for `overflow`,
`keyboard` for `touchKeyboard`, `pen` for `penMenu`, `touchpad` for
`virtualTouchpad`, and `input`/`language` for `inputIndicator`.

Icons render at their native size unless you set explicit button sizes, and
each icon has fine X/Y nudge settings. A tall column of native-size icons can
overhang a single-height taskbar; set the icon width/height to about 16 px if
you want it to fit.

## Position

The group can sit in the hidden-icons or Emoji column, or lease a dedicated
tray column before the notification icons, before Wi-Fi/volume/battery,
before or after the clock, or after the Show Desktop strip. The lease is
marker-tracked and fully reversible on unload.

Two **experimental** positions relocate the group out of the tray entirely:
**Left of Start** and **Right of Start** place it beside the Start button and
push the task list right to reserve room. The group follows Start as the
taskbar re-centers. Primary taskbar only.

## Detection

Icons are identified by their stable Segoe Fluent glyphs with
language-neutral fallbacks to accessibility metadata. **Force MainStack**
allows the complete native `MainStack` to participate as the `emoji` item
when Windows doesn't expose useful metadata.

## Known limitations

- The utility flyouts (the Emoji panel, the hidden-icons overflow) are
  positioned by Windows itself from the icon's location; at extreme
  screen-edge positions they can open partially off-screen. Prefer the
  tray positions if this bothers you.
- Left/Right of Start are experimental. The centered taskbar re-flows with
  an animation, and the group can briefly sit at a stale position until
  the taskbar's next layout pass settles it.
