# SystemTrayFrameGrid became a StackPanel — KB5129195 / 26200.9457

Investigated 2026-09-18 from issue
[#5530](https://github.com/ramensoftware/windhawk-mods/issues/5530) and the
reporter's attached detailed debug log. No code changed, nothing pushed.

## The break, in one line

On Windows 11 26200.9457 (2026-09 security update, KB5129195) the taskbar's
system-tray panel **keeps the element name `SystemTrayFrameGrid` but is no
longer a `Grid` — it is now a `StackPanel`.** Every lab mod reaches it with
`.try_as<Grid>()`, gets `nullptr`, logs "not found / unavailable", and returns.

The mod loads, hooks resolve, nothing is wrong with symbols or the XamlRoot.
It simply finds no injection parent and silently does nothing.

## Evidence

**1. The reporter's log (taskbar-folder-menus 0.7, build 26200.9457).**
It runs clean the whole way and dies on one line:

```
21:35:09.881  Path: C:\Windows\System32\Taskbar.dll  Version: 10.0.26100.9444
21:35:09.881  Using symbol cache pdb_483327DB5FAC66CA1D844F9D8F5054CB1
21:35:09.883  To be hooked ...: TrayUI::StartTaskbar
21:35:09.883  Applying hooks
21:35:09.893  [taskbar-folder-menus] [2202:ApplyAllSettings]: [Apply] SystemTrayFrameGrid unavailable
```

Line 2202 of the published 0.7 source is exactly:

```cpp
auto gridParent = FindChildRecursive(root, [](FrameworkElement fe) {
    return fe.Name() == L"SystemTrayFrameGrid";
}).try_as<Grid>();                       // <-- returns null on 26200.9457
if (!gridParent) {
    Wh_Log(L"[Apply] SystemTrayFrameGrid unavailable");
    return;
}
```

All five settings blocks read correctly, all four symbols resolved, the hook was
applied. `GetTaskbarXamlRoot` succeeded (no "[Apply] GetTaskbarXamlRoot failed")
and root content existed. So the walk reached the tree and the *name* matched
far enough to get to the cast — the failure is the type, not discovery.

**2. Independent confirmation — Taskbar Styler, issue #5526.**
helios-kc fixed their theme by changing the selector to
`StackPanel#SystemTrayFrameGrid, Grid#SystemTrayFrameGrid`. Same name, new type,
and both forms must be supported because older builds still use `Grid`.

**3. Independent confirmation — a merged upstream fix.**
`taskbar-ai-quota` v1.6.5 (PR #5510, merged 2026-09-17) carries this comment
and code:

```cpp
auto trayPanel = trayFrame.try_as<Panel>();
auto trayGrid  = trayFrame.try_as<Grid>();
// Newer Windows taskbars keep the name but use a StackPanel. Don't assume other
// panel types share its child-order layout semantics.
if (!trayPanel || (!trayGrid && !trayFrame.try_as<StackPanel>())) { ... }
...
if (trayGrid) { /* ColumnDefinitions + Grid::SetColumn, as before */ }
else          { trayPanel.Children().InsertAt(0, quota); }  // order is layout
```

**4. Same-build cluster.** #5528 (Fluent Media Player — tray positions fail,
taskbar positions work), #5559 (Taskbar height), #5533, #5538 all land on
26200.9457. The tray subtree is what changed.

## Blast radius in this lab

Four shipped mods hard-fail on the same cast, plus the shared template:

| Mod | Site | Effect on 26200.9457 |
|---|---|---|
| Taskbar Folder Menus | `taskbar-folder-menus.wh.cpp:3447`, published 0.7:2202 | Reported. Toolbar never appears. |
| Privacy Indicator Anchor | `privacy-indicator-anchor.wh.cpp:4680` | "SystemTrayFrameGrid not a Grid"; no anchor. PR #4843 open. |
| VD Switcher | `taskbar-vd-switcher.wh.cpp:4108` (`FindLiveSystemTrayFrameGrid` returns `Grid`) | Buttons never inject. PR #4844 open. |
| Tray Utility Customizer | `tray-utility-customizer.wh.cpp:3177` | "SystemTrayFrameGrid not found"; no customization. |
| `_templates/injected-grid-column.h` | whole template | Column-lease model assumes a `Grid`. |

Jax765 confirms Folder Menus on #5530 ("icon just doesn't show up"); Vc-86
reports the same build.

## Fix as implemented (2026-09-18, awaiting live test)

`_templates/injected-grid-column.h` v1.3 now leases a **slot** on the `Panel`
base instead of a column on a `Grid`:

- `Classify()` returns `Columns` (Grid), `Order` (StackPanel) or `Unsupported`.
  Anything else is refused and logged with its real class name — never guessed.
- `ResolveSlot()` (was `ResolveColumn`) yields a column on a Grid and a child
  index on a StackPanel, from the same named anchors.
- `AcquireAt()` inserts a ColumnDefinition on a Grid, or inserts the marker at
  an index on a StackPanel, where no column is created or owned.
- `PlaceChild()` and `Release()` fork internally, so call sites stay type-free.
- `DescribeChildren()` lists the tray's named children for failure logs.

Classification happens on every injection, never cached: `StartTaskbar` rebuilds
the tree and older builds still ship a Grid.

Per-mod: Folder Menus, Privacy Anchor and Tray Utility carry the refreshed
template verbatim (parity verified) and their tray variables widened to `Panel`.
Tray Utility additionally needed real work — its host markers recorded a
*column* to restore, which means nothing on an ordered panel, so a `HostRecord`
now records whether the tray is ordered, captures the marker at the host's
index, and `ReturnHostToTray()` re-inserts each host at its marker's live index
instead of appending and rewriting a column. VD Switcher does not embed the
template and got the same fork inline (`ClassifyTray`, `IndexOfTrayChild`,
`DescribeTrayChildren`), including its secondary-taskbar and rebuild paths;
its rebuild already removed and re-inserted at the same index, which is
order-correct as-is.

All four compile and link with Windhawk's bundled clang, pass the exit-time
destructor audit and template parity, and pass the upstream PR validator —
except Folder Menus' expected "version 0.7 already used" warning, since the
policy reserves the bump for the PR commit.

Not verified, and the reason the new logs exist: whether `MainStack`,
`NotifyIconStack`, `ControlCenterButton`, `NotificationCenterButton` and
`ShowDesktopStack` are still direct children of the renamed panel. If any moved,
anchor resolution fails gracefully and the log now prints the panel's class plus
its actual named children, which settles it in one run. `RootGrid` is assumed
unchanged; all four mods still cast it to `Grid`.

## Shape of the fix (original analysis)

The anchor logic already keys off *named direct children*
(`ControlCenterButton`, `NotificationCenterButton`, `ShowDesktopStack`,
`NotifyIconStack`, `MainStack`), which is the part that survives. Only the
**insertion mechanic** has to fork:

- Resolve the element to `Panel` (the common base), not `Grid`.
- Accept `Grid` or `StackPanel`; on anything else, log the real class name via
  `winrt::get_class_name()` and bail rather than guess.
- `Grid` path: unchanged — insert a `ColumnDefinition`, shift columns/spans,
  `Grid::SetColumn`.
- `StackPanel` path: no column exists. Anchor position becomes a **child
  index** — `Children().IndexOf(refElem)` (+1 for "after") and
  `Children().InsertAt(index, ...)`. Layout follows child order.
- Teardown forks the same way: on a `StackPanel` there is no leased column to
  return, only a child to remove.
- Keep both paths. Older builds still ship a `Grid`, and the rollout may be
  staged, so type must be detected at runtime every time — never cached across
  a `StartTaskbar` rebuild.

Unverified and worth checking on a live 9457 tree with UWPSpy before coding:
whether `MainStack` / `NotifyIconStack` / `ControlCenterButton` remain direct
children of the renamed panel at the same nesting level, and the panel's
`Orientation`. Tray Utility and Privacy Anchor depend on those names as *direct*
children; Styler's report suggests inner names survived, but that is inference,
not observation.

## Consequence for the parked test batch

The user's Sept 9 "works" confirmations were made on 26200.9278 — **before**
KB5129195 (applied Sept 14–15). The lab machine is now on 26200.9457. Every
tray-injecting candidate in `outputs/test-candidates-2026-09-11` therefore needs
re-validation on the current build; those confirmations no longer describe the
OS the mods will run on.
