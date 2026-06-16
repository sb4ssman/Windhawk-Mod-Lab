# PR Note

This adds a small companion mod for Taskbar Clock Customization. It introduces
an elastic `%s%` spacer token for top/bottom clock line formats, allowing dense
multi-line clock layouts with left, right, center, and distributed alignment.

The mod is intentionally standalone and limited in scope. It relies on Taskbar
Clock Customization for the actual clock text and styling, then replaces lines
containing `%s%` with equivalent XAML rows using star-sized spacer columns.

Known limitation: `%s%` is handled after Taskbar Clock Customization expands its
format tokens, so it works in the top/bottom line formats but not inside
generated composite segments such as the weather string.

I would also be happy to see this token absorbed into Taskbar Clock
Customization itself if it fits that mod's direction. I kept this companion mod
small so the behavior can be reviewed independently first.
