# Working Notes - Windhawk Mod Lab

## TOP HANDOFF - future settings UI, then return to Windows 11

Keep [_templates/dynamic-setting-options.h](../_templates/dynamic-setting-options.h)
and [its adoption guide](../_templates/recipes/windhawk-2-dynamic-settings.md)
ready for the new Windhawk release. Installed stable is 1.7.3: the current VD
candidate uses separate Win10/Win11 position lists. Recheck official release
and schema and test the new UI before family-wide adoption.

User direction: finish the surgical Win10 VD Switcher experiment here, then
return to Win11 to finish fixing/testing and publishing the family. Oct 6
follow-up explicitly requires clock/notification placements and reliable
space restoration, superseding the previous placement deferral.

## Win10 experimental checkpoint accepted - return to Windows 11

User accepted a3c43a1 on Oct6, supplied the final grid screenshot, and explicitly
authorized README updates, commit and push to the lab. Current Win10 pass is
complete. Keep support experimental; appearance and exhaustive side/lifecycle
coverage remain limitations. Do not expand this pass unless requested.
Header stays2.1. This lab push does not update any upstream Windhawk PR.

The Win10 screenshot is assets/win10-experimental-grid.png, at the top of
the root catalog, mod README and embedded Windhawk README. It demonstrates
three desktops plus enabled in-grid Task View, four equal cells.
Reference: [implementation notes](knowledge/taskbar-vd-switcher-win10-tray-reservation-2026-10-06.md)
and [real-font regression](tools/test-vd-native-layout.py).

## Windows 11 family next

Resume 2.1 native-edge fixes and fresh live tests. VD Switcher needs a Win11
regression test, including hover previews (shared native preview helper).
Use [the Oct 4 guide](outputs/live-test-2026-10-04.md), reconciled against the
latest literal-layout rule: written shapes, nudges, offsets and screen-named
positions are literal; only auto adapts. Tree-dump tool publishes with the
family after its live test. Preserve identities and the human-test push gate.

Older detail: [preserved handoff](knowledge/lab-handoff-before-win11-return-2026-10-06.md)
and [work log](work_log.md); reconcile git/PR state before trusting old claims.
