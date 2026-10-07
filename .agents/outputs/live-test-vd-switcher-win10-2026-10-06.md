# Windows 10 VD Switcher: tray placement and space restoration retest

Replace the complete source in Windhawk with
taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp and compile. Back up settings
from Textual mode first. This is a local experimental candidate for Windows
10 Home 22H2 build 19045.7725 on Windhawk 1.7.3. User accepted a6a7917 for personal experimental use. The NEW columns-first
compact automatic grid still needs an exact-build human live test. New init log: Classic Windows 10 tray backend selected.

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
   The toolbar backdrop should match the real taskbar, including accent and
   transparency; idle buttons should have no gray tiles. Default active button
   has a subtle accent tint/underline; hover and press are softly translucent.
   Click the center and edges of an idle button, not only the number; check
   name tooltips, previews and switching still work. Test an OS accent change
   without restarting the mod. High contrast follows system colors.
   First use Arrangement `auto`, Fill columns first, default 22 px height,
   default font, 2 px spacing and zero padding. Two desktops should stack in
   one column on a 40 px taskbar; four should form a 2x2 grid. Increase font
   size to verify it does not cram unreadable rows. Test vertical padding and
   Task View slivers; automatic compaction must reserve their space first.
   Increase taskbar height and check more lines fit; on side taskbars, width
   is the constraint. Fill rows first should retain the configured sizes.
   For a MANUAL stack, Arrangement `1, 2`, button height 18 px, spacing
   2 px, vertical padding 0: expected group height 38 px on a 40 px taskbar.
   For four, use `1, 2 | 3, 4`. Test `auto` and resize the taskbar: rows should
   refit while manual arrangements keep their written shape.
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
