# Taskbar Folder Menus

A Windows 11-only mod that adds compact taskbar buttons which open Shell targets
as native popup menus, recreating the most useful part of the classic taskbar
toolbar workflow. Windows 10 is not supported.

Folder buttons form one grouped toolbar at a shared taskbar position.
Independent per-folder placement around the taskbar is outside this mod's scope.

![Two folder buttons in the system tray](assets/desktop-controlpanel.png)
*A minimal two-button setup for Desktop and Control Panel.*

![Folder button destination tooltip](assets/tooltip-shows-destination.png)
*Hovering a compact button shows its configured Shell target.*

![Four folder buttons on a standard taskbar](assets/c-github-desktop-controlpanel.png)
*Drive, GitHub, Desktop, and Control Panel shortcuts arranged in a grid.*

![Four folder buttons on a taller taskbar](assets/c-github-desktop-controlpanel-v.png)
*The same four shortcuts arranged vertically by the grid layout on a taller taskbar.*

![Control Panel opened as a native Shell menu](assets/controlpanel-menu-open.png)
*The Control Panel namespace opens directly as a native Shell menu with full icons.*

![Whole drive opened from a taskbar folder button](assets/whole-drive-on-taskbar.png)
*A drive-root button opens the whole drive as a native cascading Shell menu.*

Click a small button to browse a folder, drive, Desktop, or Control Panel
directly from the taskbar — no minimizing required. Subfolders expand on
hover. Right-click any item for the full classic Windows Shell context menu.
Every folder popup includes an "Open in Explorer" shortcut at the top,
including the configured root folder.

## Example folder entries

```text
Label: 🖥    Target: shell:Desktop
Label: ⚙     Target: shell:ControlPanelFolder
Label: 📥    Target: %USERPROFILE%\Downloads
Label: C:    Target: C:\
```

Add one record per button in the Folders setting. Each record has a short button
label and a target. Targets can be normal paths or Shell namespace roots like
`shell:Desktop` and `shell:ControlPanelFolder`. Environment variables such as
`%USERPROFILE%` are expanded automatically. The full label and target appear in
the tooltip.

Emoji labels are a natural fit for narrow buttons. Label ideas: 📁 folder,
🖥 desktop, 💻 laptop, 🪟 windows, 📥 downloads, 🌐 network, 🗄 drive,
📄 documents, 🔧 tools, ⚙ settings, ⭐ favorites.

## Reordering folder buttons

For several existing records, open the mod's Settings page and switch to
**Textual mode**, then move each complete `Label` + `Target` + `UseDefaultIcon` record as a block.
The regular form currently provides add/remove controls but no direct
drag-to-reorder control.

## Native folder icons

Enable **Content → Folders → Use native Shell icon** for any entry to show
its actual Shell icon. Labels remain the default and provide the fallback if
the icon cannot be obtained. Icons are cached as pixels at the requested
display size; changing settings refreshes the requested DPI size. Text color
and font size affect labels; button dimensions determine native icon size.

Icons are fetched in the background, so a slow target never holds up the
taskbar. A target that takes a long time to fail, such as a network share
that is offline, keeps its label and is not tried again until you next change
the mod's settings.

## Placement after app icons

**Placement → Position → After pinned/running app icons** places the whole
toolbar after the rendered app buttons and follows them as apps open and close.
It reserves space and stops at the tray's left edge. If the taskbar cannot fit
the toolbar, it hides until enough room is available. Other positions place the
group before notification icons, before the OmniButton, before/after the clock,
or after Show Desktop. This mod targets the primary horizontal Windows 11 taskbar.

## Layout

**Layout → Arrangement** defaults to `auto`, which fits the available taskbar
height and logs its equivalent expression. Use folder numbers from the list:

| Expression | Result |
|---|---|
| `1 \| 2 \| 3` | One row |
| `1, 2, 3` | One column |
| `1, 2 \| 3, 4` | Two columns of two |
| `(1 \| 2), 3` | Two buttons above a third |
| `1 \| pad \| 2` | An empty button-sized space between entries |
| `1[2,-1] \| 2` | Nudge the first button right 2 DIP and up 1 DIP |

`folder1` is an alias for `1`. Unknown or unavailable items occupy no space;
duplicate folder tokens show one button. Invalid syntax logs its position and
falls back to `auto`. Unlisted folders are appended automatically unless
**Layout → Unlisted folders** is set to Hide.

## Settings

Dimensions use device-independent pixels (DIP) and scale with Windows display scaling.

| Group / setting | Default | Description |
|---|---|---|
| Placement.Position | `beforeIcons` | Position of the single toolbar |
| Content.Folders | Desktop, Control Panel | Each entry has Label, Target and UseDefaultIcon (off) |
| Content.DefaultLabel | 📁 | Used for empty labels |
| Layout.Arrangement | `auto` | Expression described above |
| Layout.FillOrder | `rows` | Automatic rows-first or columns-first filling |
| Layout.Justify | `center` | Align items along the cross axis: start, center, end |
| Layout.NewItems | `append` | Append unlisted entries or hide them |
| Size.ItemWidth / ItemHeight | 24 / 22 | Button dimensions, 10–256 DIP |
| Size.ItemSpacing | 4 | Gap, 0–80 DIP |
| Adjust.PadX / PadY | 0 / 0 | Symmetric reserved padding, 0–80 DIP |
| Adjust.OffsetX / OffsetY | 0 / 0 | Cosmetic group translation, −80–80 DIP |
| Surface.FontSize | 10 | Label size |
| Surface.TextColor / BackgroundColor | empty | System foreground/background |
| Surface.HoverBackgroundColor | `accent` | Hover background |
| Surface.PressedBackgroundColor / BorderColor | empty | System pressed background/border |
| Surface.BorderThickness / CornerRadius | −1 / −1 | System default; 0 removes border or makes corners square |
| Surface.Opacity | 100 | Button opacity, 0–100% |
| Surface.ShineEffect | off | Gradient highlight on custom background colors |
| Behavior.MaxMenuItems / MaxDepth | 150 / 0 | Limits each menu to 150 items by default; 0 remains unlimited. Submenu depth is unlimited by default. |
| Behavior.ShowHidden | off | Include hidden items; protected operating system files follow Explorer's own setting |

All color settings accept `#RRGGBB` or `#AARRGGBB` hex (the alpha byte is
honored), the generics `accent`, `accentLight`, and `accentDark` for the
Windows accent shades, or `transparent` for a fully transparent surface —
nothing drawn, button still present and clickable, useful for borderless
background-free buttons. Leaving a color empty keeps the system default for
that state.

## Note on shell:Desktop

`shell:Desktop` shows the full Desktop Shell namespace — user shortcuts, public
shortcuts, and virtual items like Recycle Bin — not just the physical Desktop
folder. Duplicates from the user+public Desktop merge are suppressed automatically.

## Upgrading from 0.7

2.0 reorganised every setting into the groups above, and Windhawk cannot
carry a value across a renamed setting. **After updating, your buttons return
to the defaults (Desktop and Control Panel) until you re-apply your settings
once.** Before updating, copy your settings from the mod's Settings page in
**Textual mode**; afterwards, re-enter your folders under Content → Folders,
preserving their order and targets, and copy appearance and menu preferences
into their corresponding groups. Defaults remain 24×22 buttons, 4 DIP spacing
and 10 DIP labels; menus are now limited to 150 items per folder by default
(Behavior → Max menu items, 0 for unlimited).

Replace single-row/single-column settings with `1 | 2` / `1, 2` (extend for your
folder count), or use `auto`. The former separate left/right padding controls
become symmetric PadX; OffsetX provides a cosmetic horizontal adjustment.

Native-icon and after-app-icon feature ideas were contributed by
[diegoalejo15](https://github.com/diegoalejo15). This implementation adds icon
caching, a tray-edge limit, and shared layout, settings, surface and tray-column components.
