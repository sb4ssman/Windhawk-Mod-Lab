# Windows 10 VD Switcher: tray placement and space restoration retest

Replace the complete source in Windhawk with
taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp and compile. Back up settings
from Textual mode first. This is a local experimental candidate for Windows
10 Home 22H2 build 19045.7725 on Windhawk 1.7.3. Exact replacement not yet
human-live-tested. New init log: Classic Windows 10 tray backend selected.

1. Create three desktops. In Placement use Position (Windows 10 experimental),
   not the Windows 11 list. Test all three choices:
   - Before hidden-icons chevron.
   - Between clock and notifications.
   - Between notifications and Show Desktop.
   Check the clock remains readable and clickable, notifications opens, and
   Show Desktop still works. No overlap, wrapping or extra blank gap.
2. Cycle those choices ten times; resize buttons and change arrangements.
   Only the current switcher's width should be reserved. Disable: all native
   positions and app-button space should return. Re-enable and repeat.
3. Click every desktop; verify switching/highlight. Switch with Ctrl+Win+arrows;
   create, rename and remove desktops in Task View. Updates take about 250 ms.
4. Test previews and dismissal on mouse leave/click. Disable previews for name
   tooltips. Enable/click Task View button. Disable with a preview open.
5. Test hide when single: reduce to one desktop, verify space returns; create
   a second and verify the chosen placement returns. Repeat at each position.
6. Test a normal and double-height taskbar, light/dark themes and DPI changes.
   Side taskbars are experimental and need separate placement verification.
7. Confirm Windows 11 position choices do not affect Win10. Restart Explorer
   when convenient and verify the selected Win10 position rebuilds once.

The old orphan-band reservation on this live shell has been recovered. The
replacement uses the native clock's minimum-size hook and sibling tray HWND;
it never inserts rebar bands. If native clock symbols cannot be hooked, init
fails; capture the log. If the clock or required notification host disappears,
attachment retries without overlapping native controls.

Primary taskbar only; no Start overlay, XAML styling or acrylic transparency.
Manual arrangements larger than the tray can clip. No push until the user
confirms this exact candidate works live; compile success is not approval.
