# Taskbar orientation — how Windows and other modders handle it (2026-10-04)

Research for the 2.1 pass (every mod works on any taskbar edge). Sources: the
upstream `mods/` tree at `bb8f7a13` (2026-10-04) and upstream issues. Nothing
here is live-verified on the user's machine yet.

## What Windows now does

- The September 2026 update adds a native taskbar position setting (Settings →
  Personalization → Taskbar → Taskbar behaviors): Bottom, Top, Left, Right. Gated
  by feature flag **59213768** and rolled out per PC.
- The chosen edge is stored as `HKCU\...\Explorer\Advanced\TaskbarLocation`
  (`ABE_*`: 0 left, 1 top, 2 right, 3 bottom). The user's machine read 3 on
  2026-10-04 after moving it back to the bottom.
- Native side taskbar width is fixed by Windows (48, or 160 with labels, per
  m417z's settings text). Horizontal gets a default and a small height.
- **Windows announces orientation in XAML visual states**, per taskbar:
  - Taskbar root grid (parent of `TaskbarFrameRepeater`): group `DockingStates`,
    states `DockedLeft` / `DockedRight` (and presumably top/bottom).
  - `SystemTrayFrameGrid`: group `OrientationStates`, state `VerticalOrientation`.
- On a side taskbar the notification icons sit in a native **`WrapGrid`**
  (`ItemWidth`, `ItemHeight`, `MaximumRowsOrColumns`) inside
  `SystemTray.SystemTrayFrame`, whose `Width` is the frame width.
- Native top taskbar keeps running indicators at the bottom of the bar (screen
  center side). Windows does NOT mirror; m417z offers mirroring as an option.

## The maintainer's position (m417z)

- **Native first.** Vertical Taskbar 1.4+ and Taskbar on Top 1.2 use the native
  implementation by default (`useNativeTaskbar: true`) and customize it; their
  own rotate-the-horizontal-taskbar implementation is now a fallback. He called
  that fallback "not an optimal option long-term" (#5794).
- **Mods may resize the side taskbar.** Vertical Taskbar sets the native side
  taskbar's width (`TaskbarWidth`, default 80) and resizes task buttons to it.
  Our layouts must expect any width.
- **Secondary taskbars can be on different edges** (`taskbarLocationSecondary`
  in both mods). Edge is per taskbar, not global.
- **Clock on a native side taskbar is auto-sized**: the clock container height
  setting is "Not used with the native vertical taskbar"; "it should be
  auto-adjusted now" (#5761). His answer for fitting content: format the clock
  in Taskbar Clock Customization, e.g. `%n%` newlines.
- **Rapid adoption across his catalog** (Sept 18 – Oct 2): tray icon spacing and
  grid 1.4, Start button always on the left 1.3.3, Multirow 1.1.4, Labels.

### How his mods translate layout to a side taskbar

- **Tray icon spacing and grid 1.4**: settings are reinterpreted along/across —
  "Tray icon width" becomes the icon HEIGHT, "Tray icon rows" becomes the number
  of COLUMNS. His own arrangement-order option is "Not used with the native
  vertical taskbar"; he sets the native WrapGrid's size and column count and lets
  Windows place the icons. Columns = min(setting, frameWidth / itemWidth).
- **Start button always on the left 1.3.3**: "With a vertical taskbar, the
  buttons and the Start menu are moved to the top instead." Implemented with
  `LeadingMargin(vertical) = Top : Left`, `Extent = Height : Width` — along-axis
  helpers, leading = top. Reading order, not rotation.

## Other modders

- **Taskbar Separators (digART)**: `orientation` setting auto / horizontal /
  vertical; auto infers direction from the positions of adjacent realized icons.
- **Taskbar Dock Animation Plus (incconutwo)**: resolves a per-taskbar edge
  (Left/Right/Top/Bottom) from the taskbar window, aspect-ratio fallback; uses
  ActualHeight vs ActualWidth as the along extent; offers "disable vertical
  bounce" for side bars.
- Only m417z's mods (plus desktop-draggable-widgets) read the native signals
  (visual states / `TaskbarLocation` / the feature flag). Everyone else infers.

## What this means for the family

1. **Detection**: replace the aspect-ratio test in `taskbar-metrics` with the
   per-taskbar `DockingStates` / `OrientationStates` visual state — Windows' own
   statement of the edge, available per taskbar, including secondaries. Keep
   aspect ratio only as a fallback. With m417z's mods now native by default, the
   RenderTransform-rotation conflict that justified standing down only exists
   when a user turns `useNativeTaskbar` off.
2. **The along/across model is the ecosystem's model too.** Decided Oct 4:
   follow m417z exactly — along = reading order (leading = left/top), no
   mirroring across the bar. Side bars swap along and across; top = bottom.
3. **Work WITH native panels where they exist** (the side-tray WrapGrid), as
   m417z does, instead of positioning over them.
4. **Width is variable** on side bars (other mods resize it); measure every time.
5. Watch for the edge changing live — m417z hooks `TrayUI::_StuckTrayChange` /
   `_HandleSettingChange`; a visual-state change on the root grid is the XAML
   equivalent. Whether a move rebuilds the taskbar or relays it out is still
   unverified (the tree-dump diagnostic answers it).

## Tree-dump findings (2026-10-04, 26300.9550, one monitor, 100% scale)

Dumps in [tree-dumps/](tree-dumps/), taken with the lab's taskbar-tree-dump
tool at bottom (96, 48, 32 px tall), top, left, right.

1. **A move is a re-layout, not a rebuild.** The same root element survived
   every move, `TrayUI::StartTaskbar` never fired. Mods must detect the change
   themselves; the rebuild hook will not tell them.
2. **The edge signal:** `Grid #RootGrid` (child of `Taskbar.TaskbarFrame`) has
   `DockingStates` = `DockedBottom` / `DockedTop` / `DockedLeft` /
   `DockedRight`, correct in every dump. Per-element `OrientationStates` are NOT
   reliable — task-button `IconPanel`s and `OverflowToggleButtonRootPanel` kept
   `VerticalOrientation` after returning to the bottom. Read RootGrid only.
   `SystemTrayFrameGrid` also carries `OrientationStates` and
   `LayoutStates` (`NormalLayout` / `VerticalLayout`), correct in every dump.
3. **Thickness:** `TaskbarFrame` has explicit `H=48` (bottom/top) or `W=160`
   (sides). Horizontal heights seen: 96, 48, 32. Side width: 160.
4. **Tray anchors survive unchanged.** `SystemTrayFrameGrid` stays a
   StackPanel with the same children in the same order (NotifyIconStack,
   NotificationAreaIcons, MainStack, NonActivatableStack, SecondaryClockStack,
   ControlCenterButton, NotificationCenterButton, ShowDesktopStack); it flips to
   `stack=V`, `va=B`. Windows swaps padding along/across itself
   (`0,4,0,4` → `4,0,4,0`).
5. **Windows pins heights on side bars:** NotifyIconStack `H=38`,
   ControlCenterButton (OmniButton) `H=38`, NotificationCenterButton (clock)
   `H=44` — the 44 is what clips a multi-line clock (Styler target).
6. **OmniButton's items host is swapped in place:** horizontal `StackPanel`
   (spacing 4) → `WrapGrid` (itemW 53, itemH 38, max 3 per row). Battery switches
   `BatteryLayoutStates` `DefaultLayout` → `DetailedVerticalLayout` (glyph over
   percent, `BatteryStackPanel stack=V`). Notification icons: StackPanel →
   WrapGrid (itemW 53, itemH 44, max 3).
7. **No mirroring.** Left and right are structurally identical apart from the
   background stroke's side; Start is at the top, tray at the bottom on both.
   Decision (user, Oct 4): follow this — no mirroring. Side bars swap along
   and across (`|` runs down, `,` across, nudges `[dx,dy]` → `[dy,dx]`); top
   is identical to bottom.
8. Start/task buttons: `TaskbarFrameRepeater` (ItemsRepeater) runs top→bottom
   on side bars, Start at the top.

## Open questions for the diagnostic

- The full `DockingStates` state names (top/bottom).
- Where MainStack / ControlCenterButton / NotifyIconStack / ShowDesktopStack sit
  on a side taskbar, and whether the OmniButton inner panel changes orientation.
- What gives the clock its height on a native side taskbar (the user's clock
  clips with multi-line Taskbar Clock Customization output, without m417z's
  Vertical Taskbar installed).
- Rebuild vs relayout on a live edge change.
