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

## Native appearance follow-up

User reports placement success but notes gray toolbar/button backgrounds.
The classic painter incorrectly reused the popup palette's solid chrome.
Replacement uses a WS_EX_LAYERED child and UpdateLayeredWindow (Win8+ child
support), so Windows composites over the real taskbar instead of an estimated
wallpaper/acrylic color. Local Surface owns two temporary 32-bit DIB/DCs;
grayscale GDI coverage is composed to premultiplied BGRA. Alpha 1/255 over
the hit region avoids color-key/zero-alpha click-through. Explicit text colors
and custom surfaces remain available; default idle clear, active accent40/255
plus underline, hover24/255, press42/255. High contrast prioritizes OS colors.
CurrentTheme is compared every existing worker update (250 ms). Native
position/reservation code unchanged; popup remains an opaque themed popup.

Official API evidence:
https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-updatelayeredwindow
https://learn.microsoft.com/en-us/windows/win32/winmsg/window-features

Stack/grid support already shared with Win11: native HWND height feeds
AvailableRows and ComputeButtonPlacements, with literal manual expressions.
Two 18 DIP cells plus 2 DIP spacing total38 DIP. Automatic default 22 DIP
cells fit only one row on a 40 DIP taskbar; decrease height or grow taskbar
to permit two, rather than silently scaling a user's written layout.

## Columns-first automatic compaction

After experimental acceptance, user requested more aggressive automatic
gridding. Existing native metrics were correct but 2*22+2=46 DIP exceeded40.
classic_ui::AutoCell measures real font bounds and computes a preferred-max
cross-axis cell size via CompactExtent and the shipped ngl::ChooseShape.
Only Win10 Arrangement=auto + FillOrder=columns passes this optional cell
size to ComputeButtonPlacements/AvailableRows. Both token resolution and
capacity use the same effective dimensions, including Task View in-grid.
Manual expressions and all other callers pass no override, preserving their
shape and dimensions. DPI converts measured physical font bounds to DIPs.
Two default-font cells on40 DIP normally become19 DIP each, with2 DIP gap;
larger text/reserves can prohibit two lines. Side cross-axis is label width,
with remeasurement after content changes. Retest this candidate before
considering the Win10 pass complete again.
