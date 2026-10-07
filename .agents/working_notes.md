# Working Notes - Windhawk Mod Lab

## TOP HANDOFF - return to Windows 11 after the Win10 checkpoint

User direction (Oct 6): finish the surgical experimental Windows 10 VD Switcher
checkpoint here, then take work back to Windows 11 to finish fixing/testing and
publishing the rest of the family. No further Win10 expansion in this pass.

Future Windhawk release preparation: [dynamic-setting-options.h](../_templates/dynamic-setting-options.h)
and [the adoption guide](../_templates/recipes/windhawk-2-dynamic-settings.md).
Keep these ready; do not adopt them family-wide or require an alpha upgrade now.
The installed stable Windhawk here is 1.7.3. Recheck the official release/schema
when returning to this work; test the new UI before adopting dynamic dropdowns.

## Immediate Win10 check before calling the experiment verified

Recompile [VD Switcher](../taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp) in
Windhawk and follow [the spacing retest](outputs/live-test-vd-switcher-win10-2026-10-06.md).
Check that every Windows 11 position choice leaves the Win10 toolbar beside
the chevron WITHOUT a gap; verify switching, previews, settings changes, hide
when single, unload and restart. Initial rendering was partially live-tested;
the surgical gap fix still awaits confirmation. No full live approval yet.
One supported Win10 location: before the hidden-icons chevron; primary taskbar
only. Clock/notification-side placement is deferred. Header stays 2.1.
The checkpoint is experimental: no release tag/version bump or push.

## Windows 11 family next

Resume the 2.1 native-edge fixes and fresh live tests. VD Switcher also needs
a Win11 regression test, including hover previews (shared native preview helper).
Use [the Oct 4 test guide](outputs/live-test-2026-10-04.md) and reconcile its
older instructions against the user's latest literal-layout rule: written shapes,
nudges, offsets and screen-named positions are literal; only auto adapts.
Tree-dump tool publishes with the family after its live test. Preserve the
user's mod identities and the no-push-before-human-test rule.

Details/history/deferred items: [preserved handoff](knowledge/lab-handoff-before-win11-return-2026-10-06.md)
and [work log](work_log.md). Check git/PR state before relying on old status claims.
