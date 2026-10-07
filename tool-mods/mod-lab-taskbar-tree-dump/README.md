# Mod-Lab: Taskbar Tree Dump

A tool for **mod authors**. It writes the Windows 11 taskbar's XAML element
tree to a file you can read, search, diff, or hand to someone helping you —
one line (or one JSON object) per element, with the layout facts that matter
when a mod has to find or move something.

It changes nothing on the taskbar. It only reads.

## Why

A live inspector shows one element at a time. A dump shows the whole tree at
once, and two dumps can be diffed: bottom against side taskbar, before and
after a Windows update, with and without another mod. That is how the Mod Lab
found out that moving the taskbar between edges re-lays out the existing tree
instead of rebuilding it, and that Windows announces the edge in a visual state.

## When it writes a dump

- About two seconds after it loads (**Dump on load**).
- Whenever the taskbar settles after a change — moved to another edge, made
  thicker or thinner, or rebuilt by Explorer (**Dump on change**). It waits
  for the taskbar to stop moving, so a dump is never taken mid-animation.
- Whenever you change any of its settings. To take a dump on demand, type a
  word into **Label**; it is added to the file name, which makes the files easy
  to tell apart (`left`, `before-update`, `with-styler`).

Disable the mod when you are done; it polls the taskbar once a second while
it is enabled.

## What each element records

- Type (runtime class) and `#Name`
- Actual size `[W x H]` and position `@x,y` relative to the dumped root
- Explicit `W`/`H`, `minW`/`minH`, `maxW`/`maxH`, margin `m`, padding `pad`,
  and non-stretch alignment `ha`/`va`
- Panel facts: StackPanel orientation and spacing, Grid row and column
  definitions (`A` = Auto, `*` = Star), a child's Grid cell and span,
  WrapGrid / ItemsWrapGrid / VariableSizedWrapGrid item size and row limit,
  Canvas position
- `RenderTransform` (translate, rotate, scale, composite, group, matrix)
- `Visibility` collapsed and `Opacity` below 1
- Every visual state group and its current state — this is where Windows
  states things like the taskbar's edge (`DockingStates` on `RootGrid`).
  Groups the template left unnamed are listed as `(unnamed 1)`, `(unnamed 2)`
- Text content, only if **Include text content** is on (see Privacy)

The file header records the time, the reason for the dump, the Windows
build, the taskbar position setting, each taskbar window's size and DPI, and
whether the taskbar was rebuilt since the previous dump. Session-specific
values such as window handles and object addresses are left out, so two dumps
diff cleanly.

## Privacy

Text in the taskbar includes window titles on task buttons, the clock and
anything other mods display. **Include text content** is off by default, and
each text element then records only its character count. Turn it on when you
need the text and are not sharing the file.

## Settings

| Setting | Default | |
|---|---|---|
| Output folder | `%USERPROFILE%\Documents\Taskbar Tree Dumps` | Environment variables are expanded; missing folders are created |
| Format | Text | Text (indented, one line per element) or JSON (nested) |
| Subtree | *(whole taskbar)* | Name of one element to dump instead, e.g. `SystemTrayFrameGrid` |
| Include text content | Off | See Privacy |
| Label | *(empty)* | Added to the next file name; changing it writes a dump |
| Dump on load | On | |
| Dump on change | On | |
| Maximum depth | 80 | Levels below the root |

File names are `taskbar-<date>-<time>-<edge>-<reason>[-<label>].txt` (or
`.json`), where the edge comes from the Windows taskbar position setting.

## Limitations

- The primary taskbar's tree only. Secondary-monitor taskbars are listed with
  their window size, but their XAML is not reached.
- The taskbar's own tree only: Start, Quick Settings and the notification
  center live in other processes.
