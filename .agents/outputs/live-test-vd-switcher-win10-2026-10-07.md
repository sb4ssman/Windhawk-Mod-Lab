# Windows 10 VD Switcher: event-driven worker live test (Oct 7)

Candidate: lab `259f032` (unchanged at HEAD `73744dc`), header 2.1.
COMPILE_OK and VD_NATIVE_LAYOUT_PIPELINE_OK on this machine. Upstream
validator step not run here (no Python 3.12 with pyyaml on this machine).

Installed before this test: exactly `a3c43a1`, the Win10 build accepted Oct 6.
Rollback: `git show a3c43a1:taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp`.
The candidate also carries `85eec2b`, the review fixes live-approved on
Windows 11 only, so this is also their first Win10 run.

What changed: the Win10 backend no longer polls every 250 ms. It waits on
registry change notifications (desktops, theme, accent, high contrast), gets
a wake-up on settings save, re-checks geometry when the clock/notification
button moves plus a 2 s timer on the bar, and keeps a hidden bar while its
anchor is missing instead of destroying and recreating it.

Install: Windhawk -> taskbar-vd-switcher -> Edit, select all, paste, compile.
Turn logging on (Advanced -> Debug logging: Mod logs) for the run.

## Must pass

1. **Desktops, from outside the bar.** Each should show within about a second,
   with no restart and no clicking the bar:
   - Win+Ctrl+Right/Left switches the highlight.
   - Task View: create a desktop, rename one, remove one, drag a switch.
   - Win+Ctrl+D (new) and Win+Ctrl+F4 (close).
2. **Clicks on the bar** switch desktops and the highlight follows.
3. **Theme.** Settings -> Colors: light <-> dark, change accent, toggle
   transparency. The bar recolors without toggling the mod. High contrast
   on/off (Left Alt+Left Shift+PrtSc).
4. **Geometry.** Resize the taskbar (single <-> double height), move it to
   top and back, change display scale if practical. The bar follows and the
   reserved space stays correct (no overlap, no extra gap).
5. **Settings save.** Change arrangement or button size and save: applies at
   once (this used to ride the 250 ms poll; it now needs the wake-up).
6. **Notifications anchor.** Position = Between notifications and Show
   Desktop. Taskbar settings -> turn the Action Center button OFF: expect
   the bar to hide with no taskbar flicker, and the log to say
   "Classic tray anchor unavailable; waiting for it" ONCE (not repeating).
   Turn it back ON: the bar returns in place.
7. **Hide when single.** Close down to one desktop: space returns. Make a
   second: bar returns.
8. **Lifecycle.** Disable -> all native space returns. Enable -> bar back.
   Restart Explorer -> bar rebuilds once.
9. **Previews.** Hover a desktop: preview shows; it dismisses on leave/click.

## Note if seen

- Any lag over about 2 s on a desktop or theme change (a missed notification
  that only the slow fallback caught).
- Accent change picked up late or only after a second change.
- Explorer CPU at idle (Task Manager): should be ~0 with the bar up.
