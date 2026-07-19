# Taskbar Clock Spacer

Adds a `%s%` elastic spacer token to the Windows 11 taskbar clock, so clock items
can be pushed apart to fill a fixed width instead of bunching together.

## Two requirements — please read before installing

**1. This mod does nothing on its own.** It is a companion for
[Taskbar Clock Customization](https://windhawk.net/mods/taskbar-clock-customization).
That mod produces the clock text; this mod only rearranges it. Install and
configure that mod first.

**2. The clock needs a fixed width.** An elastic spacer distributes *leftover*
width. If the clock sizes itself to its own text there is no leftover width,
every gap computes to zero, and the result looks exactly as if the mod were not
installed. Set a fixed width using either:

- **Max width** in Taskbar Clock Customization's settings, or
- **Max clock width** in this mod's settings.

Either one works. 120 px is a reasonable starting point.

Windows 11 only. This mod does not work on Windows 10.

## What it does

Put `%s%` between items in the clock's Top Line or Bottom Line format. Each `%s%`
becomes a gap, and all leftover width is shared out evenly between the gaps.

| Format | Result |
| --- | --- |
| `%time%%s%%date%` | time hugs the left edge, date hugs the right, gap fills the middle |
| `%time%%s%%date%%s%%weekday%` | three items, two equal gaps |
| `%time%%s%%s%%date%` | two gaps in a row, so the date is pushed twice as far right |

The first item always hugs the left edge and the last always hugs the right edge,
so the line stays anchored as the text changes width.

## Setup

1. Install **Taskbar Clock Customization** and set up your clock format.
2. Set a **Max width** in its settings, for example `120`.
3. Install this mod.
4. Edit the clock mod's **Top line** or **Bottom line** to put `%s%` between
   items, for example `%time%%s%%date%`.

The `%s%` token passes through Taskbar Clock Customization untouched and is
interpreted here at display time.

## Troubleshooting

**`%s%` disappears and nothing moves.** This is the fixed-width problem in
requirement 2 above. Set a **Max width** in Taskbar Clock Customization, or a
**Max clock width** here. The mod also writes a one-line explanation to the
Windhawk log the first time it detects this.

**Nothing happens at all.** Confirm Taskbar Clock Customization is installed and
enabled, and that `%s%` is in its **Top line** or **Bottom line** setting — not in
the tooltip, the middle line, or the weather format.

**The spacer works but the clock is the wrong width.** Adjust the same Max width
value. Use **Line width override** only if the automatic width is being read
incorrectly.

## Settings

- **Line width override** — explicit width for the spacer grid. Usually `0`
  (automatic) is correct; the width is inherited from the clock's Max width.
- **Max clock width** — applies a hard maximum width to the clock text, the
  generated spacer rows, and their immediate clock panel. Use this if you prefer
  not to change Taskbar Clock Customization's own settings.
- **Minimum spacer width** — a floor, in pixels, for every gap. `0` (the default)
  leaves gaps fully elastic. A small value such as `8` guarantees a visible gap
  even before a fixed clock width is configured.

## Limitations

- `%s%` is interpreted after Taskbar Clock Customization expands its format
  tokens, so it works in the top and bottom line formats but not inside generated
  composite segments such as the weather string.
- Lines without `%s%` are left completely alone — the mod is a no-op for them.
- Font, size, and color of the spaced segments follow the original clock text's
  current style, so the clock mod's style settings continue to apply.

## How it works

The mod hooks `DateTimeIconContent::OnApplyTemplate` in the system tray and
watches the clock's time and date text blocks. When a line contains `%s%`, the
source text block is collapsed and a generated panel is inserted in its place:
each line becomes a Grid whose text segments sit in `Auto` columns separated by
`Star` columns, and the star columns absorb the leftover width. When only the
text changes — which happens every second — the existing segments are rewritten
in place rather than rebuilt, so the visual tree stays stable.

## Files

- [taskbar-clock-spacer.wh.cpp](taskbar-clock-spacer.wh.cpp) — Windhawk mod source

## Status

Version `1.1`. Static checks pass; awaiting a live test pass and fresh
screenshots before updating
PR [#4443](https://github.com/ramensoftware/windhawk-mods/pull/4443).

## Relationship to Taskbar Clock Customization

The spacer was first offered as a patch to Taskbar Clock Customization itself
([m417z/my-windhawk-mods#68](https://github.com/m417z/my-windhawk-mods/pull/68)).
The maintainer preferred an approach that does not add generated layout elements,
so this companion mod carries the feature separately and leaves that mod
untouched.
