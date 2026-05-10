# Hit-Test Analysis — VD Switcher Intermittent Click Failures

Last updated: 2026-05-09

## Symptom

Reported by user: "half to maybe a whole button is unclickable, right after having clicked another button, but not always."

Intermittent, timing-correlated with a previous button click. Buttons are visually present and correctly rendered.

## Injection Architecture

`InjectButtonGrid` inserts a new `Auto` `ColumnDefinition` into `SystemTrayFrameGrid` at `insertCol`.
Existing children are shifted: those whose column was ≥ insertCol get col+1; those that spanned
across insertCol get their span widened by 1.

Our button grid is then appended to Children with `Canvas::SetZIndex(grid, Z)`.

In WinRT XAML Grid, `Canvas.ZIndex` controls both render order AND hit-test order for all children
of the same panel. Higher ZIndex is on top and receives pointer events first.

## Hypotheses (most to least likely)

### H1: Span-widened sibling at high ZIndex

Some system tray element (e.g. an input overlay, a background placeholder, or a transparent
pointer-capture panel) has an explicit `Canvas.ZIndex` ≥ our injection ZIndex. That element's
visual area now covers our column (because its span was widened) and intercepts clicks before
our buttons receive them.

**Diagnostic added**: Log all siblings with non-zero ZIndex at injection time (`[Inject] ZIndex sibling:`).
**Fix attempted**: Raised injection ZIndex from 100 → 10000.

### H2: Transparent hit-test surface for ShowDesktopStack hover

Windows 11 "show desktop" peek has a hover-detection zone. This might be an invisible element
that slides left to detect proximity, temporarily covering the rightmost edge of our button column
(which in `afterClock` mode is immediately left of ShowDesktopStack).

This would explain the "sometimes" / timing-correlated behavior if the peek surface appears and
disappears based on cursor position.

**Evidence**: Not confirmed. Would manifest as the right side of the rightmost button being unclickable.

### H3: Brief window during XAML full-rebuild

On full rebuild (`RebuildOrUpdate` with `fullRebuild=true` or `countChanged=true`):
```cpp
gridParent.Children().RemoveAt(idx);
g_buttonGrid = BuildButtonGrid(count, current);
// ...
gridParent.Children().InsertAt(idx, g_buttonGrid);
```
Between RemoveAt and InsertAt, no buttons exist in the tree. A click during this 1-5ms window
would miss. This only applies to count-change events (desktop added/removed), not to
simple desktop switches.

**Evidence**: Unlikely for pure desktop switching (no count change). Would be reproducible by
rapidly adding/switching desktops.

### H4: WH_CALLWNDPROC hook delivery delay

`RunFromWindowThread` installs a `WH_CALLWNDPROC` hook and then `SendMessage`s to the taskbar
window. The hook fires synchronously inside `SendMessage`, running `RebuildOrUpdate` on the
UI thread. During this execution, the UI thread is occupied and any pointer events queued
in that instant would be deferred until the hook returns.

For `UpdateHighlights` (simple switch, no count change), this should be <1ms. For a full rebuild
it could be several ms. Rapid clicking within that window would miss.

**Evidence**: Possible but window is very small for highlight-only updates.

### H5: STA COM pump during SwitchToDesktop (old, should be fixed)

The old bug: `SwitchToDesktop` was called on the UI thread, causing the STA message pump to
run and deliver the notification thread's `SendMessage` re-entrantly. This caused XAML state
corruption mid-click. **Fixed** by dispatching to a background thread.

## What We've Changed

- ZIndex: 100 → 10000 (guards against H1)
- Diagnostic log: prints all non-zero-ZIndex siblings at injection time
- Background thread for `SwitchToDesktop` (fixed H5)

## Next Steps

1. **Check Windhawk log** for `[Inject] ZIndex sibling:` lines after enabling the mod.
   If any sibling has ZIndex ≥ 10000, raise ours further or investigate that element.

2. **Try `beforeClock` or `beforeOmni` position** to see if the issue disappears.
   If it does, the problem is specific to the ShowDesktopStack adjacency (H2).

3. **Inspect live XAML tree** with a tool like XamlSpy or VisualTreeHelper log dump.
   Look for transparent panels or overlays covering the tray.

4. **Test with no other mods and minimal desktop count** to isolate whether it's
   a timing issue (H4) or an element issue (H1/H2).

## ZIndex Values in Windows 11 System Tray (known)

- Default tray elements: ZIndex = 0 (unset, collection order determines z-order)
- Our VD switcher grid: was 100, raised to 10000
- Start-overlay mode: 1000 (unchanged, used for nextToStart/aboveStart positions)

Unknown: whether any system tray overlay uses ZIndex > 0. The diagnostic log will reveal this.
