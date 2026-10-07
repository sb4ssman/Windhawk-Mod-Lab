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

## Win10 experiment accepted - return to Windows 11

User accepted a6a7917 on Oct 6 as working well enough for personal experimental
use, with appearance still imperfect. This completes the current Win10 pass;
do not expand or polish it further unless requested. Keep it experimental,
primary taskbar only; side-taskbar and exhaustive lifecycle coverage are not
established by this acceptance. Header stays 2.1; no publishing authorization.

Reference: [Win10 retest guide](outputs/live-test-vd-switcher-win10-2026-10-06.md)
and [implementation/recovery evidence](knowledge/taskbar-vd-switcher-win10-tray-reservation-2026-10-06.md).
Stacks/grids use Arrangement `1, 2` or `1, 2 | 3, 4`; 18 px height,
2 px spacing and zero vertical padding fit two rows in a 40 px taskbar.

## Windows 11 family next

Resume 2.1 native-edge fixes and fresh live tests. VD Switcher needs a Win11
regression test, including hover previews (shared native preview helper).
Use [the Oct 4 guide](outputs/live-test-2026-10-04.md), reconciled against the
latest literal-layout rule: written shapes, nudges, offsets and screen-named
positions are literal; only auto adapts. Tree-dump tool publishes with the
family after its live test. Preserve identities and the human-test push gate.

Older detail: [preserved handoff](knowledge/lab-handoff-before-win11-return-2026-10-06.md)
and [work log](work_log.md); reconcile git/PR state before trusting old claims.
