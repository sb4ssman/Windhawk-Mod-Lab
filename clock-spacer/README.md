# Taskbar Clock Spacer

Companion mod for Taskbar Clock Customization.

Adds a `%s%` elastic spacer token to clock line formats. Place `%s%` between
items and the remaining width is distributed evenly as gaps.

Examples:

```text
%time%%s%%date%
%time%%s%%date%%s%%weekday%
%s%%time%%s%
```

Edge spacers are useful too:

- `%s%content` right-aligns content.
- `content%s%` left-aligns content.
- `%s%content%s%` centers content.

## Setup

1. Install and configure Taskbar Clock Customization.
2. Set a fixed Max width in Taskbar Clock Customization, or set `Max clock width`
   in this mod to the same pixel value.
3. Add `%s%` between items in the Top Line or Bottom Line format.

## Notes

- Max width is required for visible spacer gaps.
- If no `%s%` is present, the line renders as before.
- Font and color follow the original clock TextBlock style.
- `%s%` is handled after Taskbar Clock Customization expands its tokens, so it
  works in the top/bottom line formats but not inside generated composite
  segments such as the weather string.

## Submission Note

This is intentionally a small companion mod. I would also be happy to see the
spacer token absorbed into Taskbar Clock Customization if it fits that mod's
direction.
