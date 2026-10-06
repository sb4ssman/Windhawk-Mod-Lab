Takes the native Windows 11 OmniButton — the network / volume / battery cluster that opens Quick Settings — and lets you arrange its items into any shape, hide the ones you don't want, and restyle each one independently. The button stays in its native tray position, so other mods' "before OmniButton" anchors still mean what they always did, and it uses no XAML Diagnostics, so it coexists with Windows 11 Taskbar Styler.

v1.0 was never published, so the version marks the settings contract rather than a release history: the settings are grouped (Content, Layout, Size, Adjust, Surface) around a single nestable **Arrangement** expression that replaces `itemOrder`, the grid-mode family, the coupled battery mode, and all eight per-item nudge settings. 2.1 adds Windows 11's native taskbar positions — top, left and right.

![All four items as a 2×2 block, the automatic arrangement](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/omnibutton-customizer/assets/single-height-auto.png)
![A diamond arrangement with the native tooltip](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/omnibutton-customizer/assets/single-height-diamond-adjusted-with-hover.png)
![A single column on a double-height taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/omnibutton-customizer/assets/double-height-arranged-vertical.png)
![A single row on a left taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/omnibutton-customizer/assets/side-left-single-row.png)
![Battery over its percentage between network and volume on a right taskbar](https://raw.githubusercontent.com/sb4ssman/Windhawk-Mod-Lab/main/omnibutton-customizer/assets/side-right-diamond.png)

### Highlights

- **One Arrangement field.** `auto` fits the available items to the taskbar; anything else is an expression — `|` places side by side, `,` stacks, parentheses nest, and `name[dx,dy]` nudges one item or a whole group. `auto` logs the expression it generated, so it can be pasted back into the field and edited.
- **Every taskbar edge.** The mod reads the edge Windows reports (`DockingStates` on the taskbar's `RootGrid`) and follows a move while running — a move re-lays out the existing tree rather than rebuilding it, so it watches `TaskbarFrame`'s size and the docking state instead of relying on `TrayUI::StartTaskbar`. A written arrangement is laid out exactly as written on every edge; only `auto` adapts, filling a side taskbar's width. On a side taskbar Windows hosts the items in a fixed-cell `WrapGrid`, which the mod leaves intact, translating each item from where it renders to its arranged cell.
- **Stands down on a rotated taskbar.** `taskbar-vertical` with its native mode off rotates a horizontal taskbar with a `RenderTransform` on the same elements this mod translates. The mod recognises that case — the taskbar runs down a side while Windows still reports a horizontal edge — leaves everything as found, and logs once. Its default native mode is simply a side taskbar, and is supported.
- **Only the controls each item can honor.** Network, volume and the battery are each drawn by a stack of glyphs layered on top of one another, so resizing one layer pulls it off the others. Size and font family are offered only on the battery percentage, the one item that is a single piece of text; color and opacity work on all four.
- **Recoloring works on template-bound tray glyphs.** `InnerTextBlock` inside a `SystemTray.IconView` is template-bound, so the mod styles the outermost `Control` between the leaf and the host as well.
- **Content sizing.** `Item width: 0` measures each item and reserves exactly what it needs; the percentage is always measured, so "9%" growing to "100%" cannot clip.
- **Writes no Windows settings and no registry values.**

### Testing

2.1 was live-tested on Windows 11 26300.9550 with the taskbar at the bottom, top, left and right, including moves between edges while the mod was running, with automatic and hand-written arrangements. Earlier rounds covered per-item and per-group nudges, hiding items, negative item spacing, per-item color, percentage size, single- and double-height taskbars at 100%, 125% and 150% scaling, running alongside several other taskbar and tray mods, and enable/disable/Explorer-restart teardown.

<!-- ⚠️ Please keep the template below intact and fill in the relevant sections. Any additional content can be placed above the template. -->

## Changelog

If this pull request updates an existing mod, describe the changes below:

* 2.1: support for Windows 11's native taskbar positions (top, left, right), following a move while running
* 2.1: stand down only when another mod rotates the taskbar, rather than on any vertical taskbar
* 2.1: `wifi` accepted as a name for the network item; unknown names are logged
* 2.1: the LayoutUpdated monitor compares the items host's child count instead of inferring from presenters
* Replaced the settings surface with grouped keys and one nestable `Arrangement` expression
* Fixed physical-pixel/DIP mixing in the row-capacity heuristic
* Fixed recoloring on template-bound system-tray glyphs
* Fixed the battery percentage clipping at the group edge
* Added fit-to-content item sizing and negative item spacing
* Removed the Windows battery-percentage setting write

## Mod authorship

If this pull request introduces a new mod, please complete the section below.

This mod was created by:

- - [ ] The submitter, without AI assistance
- - [x] The submitter, with AI assistance
- - [x] Claude
- - [ ] ChatGPT
- - [ ] Gemini
- - [ ] Another AI (please specify): 
- - [ ] Other (please specify): 

Please select the options that best apply. Your selection does not affect the acceptance criteria, but it helps reviewers understand the context of the code and provide relevant feedback.
