# Tray Utility Customizer

Windhawk mod for arranging Windows 11 tray utilities into one configurable row,
column, or grid.

## Current state

Version 0.3 is a guarded lab build.

It can detect:

- Show hidden icons
- Emoji and more
- Touch keyboard
- Pen menu
- Virtual touchpad
- Input/language indicator

Only controls available on the current Windows build are included. The controls
remain native; the mod moves their top-level tray hosts instead of replacing
their click handlers or flyouts.

Use the `itemOrder` setting to select and order utilities. If two selected
utilities share one indivisible Windows host, they remain bundled and the log
reports the relationship.

## Layout

- Automatic chooses a column when the tray is tall enough and a row otherwise.
- Row places utilities beside each other.
- Column stacks them vertically.
- Grid supports fixed or automatic rows and columns, row-first/column-first fill,
  and short-row/short-column alignment.

Column layouts at a native anchor reuse the proven native host positioning path.
Row and multi-column grid layouts reserve a dedicated tray column instead of
resizing a Windows-owned column. If a requested vertical layout cannot fit safely,
the mod falls back to a row.

## Position

The group can borrow:

- The hidden-icons column
- The Emoji column

Or it can insert a dedicated tray column:

- Before notification icons
- Before Wi-Fi/volume/battery
- Before the clock
- After the clock
- After the Show Desktop strip

## Useful controls

- Utility selection and ordering
- Row, column, automatic, or grid layout
- Grid rows, columns, fill order, and short-group alignment
- Tray position
- Button width and height
- Horizontal/vertical spacing
- Whole-group X/Y offsets
- Independent offsets for every supported utility
- Minimum tray height safety threshold

## Recommended test

1. Start with `overflow,emoji`, column layout, and hidden-icons position.
2. Verify both flyouts, tooltips, right-click behavior, and unload restoration.
3. Try row layout.
4. Try a dedicated position such as Before Wi-Fi/volume/battery.
5. Add any other utility enabled in Windows Taskbar settings.
6. Restart Explorer and repeat.

Force MainStack is a diagnostic fallback. It can move unrelated privacy or input
indicators if Windows bundles them in that host.

Initial testing targets the primary Windows 11 taskbar. Live testing is still
required for flyout anchoring and hit testing.
