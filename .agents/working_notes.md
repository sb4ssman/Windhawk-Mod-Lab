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

## Immediate - Win10 columns-first auto-grid candidate needs live test

The user reopened the accepted experiment: columns-first auto should attempt
a column/grid on a single-height taskbar without a manual arrangement.
Recompile VD Switcher and test auto + columns first with default sizes/font:
two desktops should stack; three plus Task View should make a 2x2 grid.
For the latter, Content -> Task View button MUST be enabled as well as
selecting last-in-grid (live registry at diagnosis had button=0, placement=inGrid).
Compact sizing uses actual taskbar height (width on side taskbars), subtracts
padding/Task View space, and uses the measured FULL font line box as a readability floor. Previous
extra 4 DIP margin blocked default Segoe UI: its real tmHeight is17 px,
so the previous21 px minimum still allowed only one line. Extra horizontal
margins also blocked higher DPI; native centering now supplies spare padding.
Configured cross-axis size is the preferred maximum. Larger fonts can still
limit it to one line. Verify resize/DPI, side orientation if convenient, and
that row-first auto and written arrangements retain exact configured sizes.
Only Win10 columns-first auto compacts; Win11 behavior remains unchanged.
Run [the real-font placement regression](tools/test-vd-native-layout.py):
it extracts AutoCell, AvailableRows and ComputeButtonPlacements from the
actual mod and checks two/three/three+TaskView at100/125/150/200% scaling.

Previous build a6a7917 was accepted for personal experimental use, with
appearance imperfect. The NEW auto-fit candidate is not covered by that
acceptance. Header stays 2.1; no publishing authorization.
Reference: [Win10 retest guide](outputs/live-test-vd-switcher-win10-2026-10-06.md)
and [implementation notes](knowledge/taskbar-vd-switcher-win10-tray-reservation-2026-10-06.md).

## Windows 11 family next

Resume 2.1 native-edge fixes and fresh live tests. VD Switcher needs a Win11
regression test, including hover previews (shared native preview helper).
Use [the Oct 4 guide](outputs/live-test-2026-10-04.md), reconciled against the
latest literal-layout rule: written shapes, nudges, offsets and screen-named
positions are literal; only auto adapts. Tree-dump tool publishes with the
family after its live test. Preserve identities and the human-test push gate.

Older detail: [preserved handoff](knowledge/lab-handoff-before-win11-return-2026-10-06.md)
and [work log](work_log.md); reconcile git/PR state before trusting old claims.
