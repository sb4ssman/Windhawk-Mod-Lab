# VD Switcher - Win10 tray reservation and recovery, Oct 6

User reported taskbar space persisting after ending/restarting Windhawk and
requested both clock/notification-side positions. Live HWND inspection found
no switcher child but ten rebar bands: nine had the switcher's private ID
0x56445357 and missing children. Native app host occupied x=162..1055 while
the rebar extended x=94..1587. RB_IDTOINDEX did not locate the private ID.
The old backend's ID lookup/insert/remove approach was therefore unsuitable.

Recovery caveat: deleting from the initial indexed snapshot also removed the
native app band, leaving its HWND hidden. This was an agent recovery mistake,
reported to the user and corrected before further work. Once empty, the rebar
was restored with its existing MSTaskSwWClass child (native ID 0, style 0xd40,
minimum width 144). Final repeat inspection: one native band, width 1493;
MSTaskSwWClass/MSTaskListWClass visible x=96..1587. Clock x=1803..1867,
notifications x=1867..1915, Show Desktop x=1915..1920. No Explorer restart.
Temporary bulk-band repair helper removed; do not preserve/reuse that unsafe
snapshot deletion approach. This does not establish the new mod's live behavior.

Replacement: reserve the switcher's extent through explorer.exe's
ClockButton::CalculateMinimumSize; shrink the actual clock HWND by that extent.
AfterClock uses the resulting gap. AfterNotifications shifts notifications
into that gap and places the switcher after it. BeforeIcons shifts preceding
tray children and the clock into the gap, leaving the reservation before the
chevron. The new mod never creates/deletes rebar bands or touches app bands.
All HWND geometry/subclasses are taskbar-thread-owned. Other clock threads
pass through. Host changes trigger reattachment; hide-single reserves zero.
BeforeUninit stops the worker, removes subclasses, clears the reservation,
and requests native WM_SIZE layout before Windhawk removes the sizing hook.

ABI and clock HWND field verified against the upstream primary source:
https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp
Clock symbol unavailable means safe init failure, not an unsafe rebar fallback.
Win10 positions use Placement.Win10Position; existing Win11 tokens unchanged.

Validation: compile/link, exit-destructor audit, assembly (8 components),
component use, settings consumption, README parity and whitespace. No human
live test of this replacement yet: Windows UI automation native pipe was
unavailable. No installed source/settings edit, release bump/tag, push or PR
action. Other family mod sources unchanged. New-release settings template
remains at the top of the handoff; adopt only after testing the released UI.
