# Windows 10 VD Switcher: surgical spacing retest

Replace the complete source in Windhawk with
taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp, then compile. Back up settings
from Textual mode first. This is an experimental local candidate for Windows
10 Home 22H2, build 19045.7725. Its initial rendering was partially tested;
the revised gap fix is not yet confirmed live.

1. Create three desktops. The toolbar should sit immediately left of the
   hidden-icons chevron. Click each: verify switching and the active highlight.
2. Select every Position (Windows 11) choice, especially left/over/right of
   Start. None should move the Win10 toolbar or leave a large blank gap.
   Position is explicitly Windows 11 only; Win10 has one supported location.
3. Switch with Ctrl+Win+Left/Right; create, rename and remove desktops in
   Task View. The buttons should update within about 250 ms.
4. Check hover previews and dismissal on mouse leave/click. Disable previews
   and check desktop-name tooltips. Enable and click the Task View button.
5. Try arrangements `1, 2 | 3` and `master | (1, 2, 3)`; change font/color/size
   settings and verify a rebuild. Check light/dark themes and normal DPI.
6. If convenient, move to all four edges and try double height. Auto refits;
   manual shapes and nudges remain screen-literal.
7. Hide when only one desktop: check space restoration and reappearance when
   another desktop is added. Also check with the hidden-icons chevron absent.
8. Disable with a preview open: toolbar/popup disappear, app space returns,
   Explorer remains usable. Re-enable and check an Explorer restart.

Deferred: between clock and notifications, and after notifications. Other
limits: primary taskbar only, no Start overlay, XAML styles, or acrylic
transparency. Oversized manual arrangements can clip/wrap.

Expected init log includes: placement is fixed before the hidden-icons chevron.
Report appearance/gap, switching, and unload results; include logs/settings for
failures. No push until the user confirms the exact candidate works live.
