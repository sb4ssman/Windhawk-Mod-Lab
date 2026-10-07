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

## Immediate: Win10 appearance candidate and stack/grid retest

Recompile [VD Switcher](../taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp) in
Windhawk; use [the placement/unload retest](outputs/live-test-vd-switcher-win10-2026-10-06.md).
Position (Windows 10 experimental): before chevron, between clock and
notifications, or between notifications and Show Desktop. Verify all three,
repeated switches/settings changes, hide-when-single and disable/re-enable.
User reports the tray placement checkpoint d88e5f1 works, but its toolbar
background/button surfaces look wrong. Recompile the new appearance candidate:
actual taskbar shows through, idle buttons are transparent, default active
accent is a subtle tint/underline, hover/press is soft, theme/accent changes
refresh automatically. No new appearance settings. Exact visuals and click
coverage still need a human live test. For stacks/grids, use Arrangement
`1, 2` or `1, 2 | 3, 4`; 18 px button height, 2 px spacing, zero vertical
padding fits two rows in a 40 px taskbar. Auto follows available height. Native clock-size symbol required; primary
taskbar only; side taskbars need testing. Header stays 2.1. No release tag,
push or PR update before the user confirms this exact build works.

Current live taskbar's old orphan bands were cleared and native app-button
width restored; see [recovery evidence](knowledge/taskbar-vd-switcher-win10-tray-reservation-2026-10-06.md).
The new backend creates no rebar bands. Do not reuse the temporary bulk-band
recovery script: it also hid the native app host during cleanup, then restored it.

## Windows 11 family next

Resume 2.1 native-edge fixes and fresh live tests. VD Switcher needs a Win11
regression test, including hover previews (shared native preview helper).
Use [the Oct 4 guide](outputs/live-test-2026-10-04.md), reconciled against the
latest literal-layout rule: written shapes, nudges, offsets and screen-named
positions are literal; only auto adapts. Tree-dump tool publishes with the
family after its live test. Preserve identities and the human-test push gate.

Older detail: [preserved handoff](knowledge/lab-handoff-before-win11-return-2026-10-06.md)
and [work log](work_log.md); reconcile git/PR state before trusting old claims.
