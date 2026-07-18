# Mod infrastructure checklist (all system-tray mods must have)

Durable engineering requirements, moved out of working_notes 2026-07-17.

- [ ] `GetModuleVersionInfo` helper
- [ ] `GetSystemTrayModuleHandle()` (3-DLL chain: SystemTray.dll → Taskbar.View.dll w/ version check → ExplorerExtensions.dll)
- [ ] `HandleLoadedModuleIfSystemTray` + `LoadLibraryExW` hook fallback
- [ ] `WindhawkUtils::Wh_SetFunctionHookT`
- [ ] `std::atomic<bool> g_systemTrayModuleHooked{false}`
- [ ] `Wh_ApplyHookOperations()` after late-hook in both `Wh_ModInit` and `Wh_ModAfterInit`
- [ ] `WaitForSingleObject(INFINITE)` on background threads in `Wh_ModUninit`
- [ ] `@compilerOptions -lversion`
- [ ] `GetTaskbarXamlRoot` uses `0x10` default + ARM64 disasm probe (NOT `0x48` + stub)
- [ ] `iconView.Loaded` uses `g_autoRevokerList` auto-revoke pattern (NOT bare lambda)
- [ ] No `%%`-unescaped `%` literals in `Wh_Log` format strings

Status (2026-07-17): omnibutton ✓, vd-switcher ✓, clock-spacer TBD,
taskmanager-tail N/A (no system tray hook), folder-menus TBD, privacy-anchor TBD.
