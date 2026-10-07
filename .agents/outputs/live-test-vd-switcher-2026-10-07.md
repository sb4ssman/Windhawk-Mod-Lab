# VD Switcher 2.1 — Windows 11 review candidate

Source: taskbar-vd-switcher/taskbar-vd-switcher.wh.cpp. No installed settings
were changed by the agent. Save your current settings before experimenting.

1. Confirm ordinary desktop switching, active highlighting, custom labels and
   creation/removal of desktops. Check both auto and a written arrangement.
2. Enable Task View and try its in-grid and sliver placement; click it.
3. Hover populated and empty desktops; check captions, thumbnails, dismissal
   after moving away/clicking, and text on a higher-DPI monitor if available.
4. Move bottom -> top -> left -> right -> bottom. Confirm literal manual layout
   and nudges, auto fitting and a preview beside a side taskbar.
5. Save placement, font and size changes; try FontSize 0 and CornerRadius -1
   in Textual mode to confirm safe clamping, then restore your usual values.
6. With a content-sized Styler taskbar, open/close windows and check the bar
   stays stable. Change taskbar thickness and confirm a layout refresh.
7. Disable/re-enable: confirm native space and Start/tray geometry restore.
   Restart Explorer if convenient: confirm the bar returns without requiring
   a desktop switch. Check secondary taskbars if you use the experimental option.

Windows 10 was previously accepted as experimental. The current real-font
regression passes; this checklist targets the outstanding Windows 11 live test.
