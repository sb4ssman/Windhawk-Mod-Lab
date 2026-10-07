# Folder Menus 2.1 candidate — live test

Load `taskbar-folder-menus/taskbar-folder-menus.wh.cpp` from the lab.

1. Open Desktop and a local folder. Check hover submenus, item opening,
   right-click Shell menus and Open in Explorer. Close and reopen repeatedly.
2. Enable native icons. Verify icons appear and labels remain the fallback
   for an invalid target. Saving settings permits another failed-icon probe.
3. Use after-app-icons placement; open/close apps and check spacing follows.
4. Check the tray positions and horizontal/side taskbar layouts, including
   an edge or thickness change. Manual arrangements should remain literal.
5. Save changed settings, then disable/re-enable. Check restoration of native
   taskbar spacing and no leftover buttons.

Shell extraction remains synchronous within the background worker. Settings
save/unload can wait for one active Shell call; cancellation occurs between
targets. Taskbar rebuild callbacks never join that worker.

Automated validation: SUBMISSION_PREFLIGHT_OK and EDGE_WATCH_SIZE_REGRESSION_OK.
Awaiting exact-candidate human live test; no push authorized by these checks.
