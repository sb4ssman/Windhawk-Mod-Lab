# Work Log — Windhawk Mod Lab

Lab-level milestones only. Per-mod version history lives in each mod's own `_claude_notes/work_log.md`.

## 2026-04-27 — Lab established

- Created Windhawk-Mod-Lab as umbrella repo
- Subtree-merged Windhawk-Vertical-OmniButton → vertical-omnibutton/
- Subtree-merged Windhawk-Taskmanager-Tail → taskmanager-tail/
- Added profiles/ folder for per-machine Windhawk config snapshots
- VD switcher design doc in place; implementation not yet started

## 2026-05-08 — Maintainer review pass (session 2)

### Vertical OmniButton → v1.4 (PR #3859 updated)
- Applied GetSystemTrayModuleHandle 3-DLL pattern
- Added GetModuleVersionInfo helper
- ARM64 arm in GetTaskbarXamlRoot
- WindhawkUtils::Wh_SetFunctionHookT throughout
- std::atomic<bool> g_systemTrayModuleHooked
- Wh_ApplyHookOperations after late-hooks
- Inline mode auto-sizing fixed
- HandleLoadedModuleIfSystemTray + LoadLibraryExW fallback

### Clock Spacer → v0.8 (in-tree, not yet committed — needs test confirmation)
- Fixed restart persistence: system tray DLL hook not surviving explorer restart
- Added GetModuleVersionInfo + GetSystemTrayModuleHandle (with Taskbar.View.dll version check)
- Added LoadLibraryExW hook + HandleLoadedModuleIfSystemTray
- g_systemTrayModuleHooked → std::atomic<bool>
- Wh_ApplyHookOperations() in Wh_ModAfterInit after late-hook
- Added -lversion to compilerOptions

### Virtual Desktop Switcher → v1.4 (in-tree, SET ASIDE — layout bug under investigation)
- All infrastructure: GetSystemTrayModuleHandle, ARM64, atomic hook flag, Wh_ApplyHookOperations
- Crash-on-disable: StopNotificationThread now waits INFINITE
- Half-button-clickable: Canvas::SetZIndex(100) on injection
- Stale parent on desktop-add: RebuildOrUpdate uses FindLiveSystemTrayFrameGrid + ApplyAllSettings fallback
- Must-move-mouse: SwitchToDesktop dispatched to background COM thread
- Sliver height: subtracted from GetAvailableRows when sliver mode active
- Layout scoring: only penalize wide layouts (cols>rows), not tall (rows>cols)
- Layout diagnostics: Wh_Log in GetAvailableRows and ComputeLayout
- Outstanding: 2-column layout on 3-desktop restart — needs log inspection to determine if height detection or scoring issue

## 2026-05-09 — Maintainer review applied (PR #3859 + #3932)

### Vertical OmniButton
- ARM64 disasm probe in `GetTaskbarXamlRoot` (default `0x48` → `0x10`, full 4-instruction pattern check)
- `iconView.Loaded` → auto-revoke pattern via `std::list<FrameworkElement::Loaded_revoker> g_autoRevokerList` (prevents crash-on-unload from stale callbacks)
- `%` → `%%` escape fix in one `Wh_Log` call
- README: `→ Settings → Advanced` → `→ Settings → Textual mode.`

### Virtual Desktop Switcher
- ARM64 disasm probe in `GetTaskbarXamlRoot` (same fix)
- `rightOfStart` position added (grid floats right of Start button)
- `aboveStart` width expansion: pushes `TaskbarFrameRepeater.Margin.Left` by `(gridW−startW)/2` to center Start under VD grid; uses `g_startButtonOriginalX` stable anchor to prevent `LayoutUpdated` oscillation
- `gridVerticalOffset` setting now applies in all overlay modes (previously only applied in tray-injection path)
- `gridVerticalOffset` + `gridVerticalOffset` struct/LoadSettings wired
- Sliver distortion fix: explicit `VerticalAlignment::Top` + computed `marginTop` replaces `Center`+compensation approach
- Fill order default: `columnFirst` → `rowFirst`
- `masterButtonSpacing`: sentinel model removed, pure additive margin offset
- Row-first short-group centering: pixel-precise via column-span + `Margin.Left` (mirrors column-first approach)

## 2026-06-17 — VD Switcher Start-placement rethink

- Collapsed `aboveStart` / `belowStart` into legacy aliases of `overStart`; settings UI now exposes only left of Start, over Start, and right of Start.
- Made overlay mode pure overlay; vertical placement is controlled by `gridVerticalOffset` instead of separate above/below placement modes.
- Reworked `rightOfStart`: pushes `TaskbarFrameRepeater.Margin.Left` to reserve room and applies a visual X counter-offset to the Start button so Start stays anchored while taskbar items move right.
- Submission-prep pass: incorporated renamed screenshots into README and Windhawk readme, documented tested left/over/right Start examples, removed stale Start-placement helper/global, and removed the Z-index diagnostic sibling dump.

### Infra checklist updated
- Added two new required patterns: ARM64 disasm probe and auto-revoke `Loaded`

## 2026-05-05 — Clock Spacer live tuning

- Clock Spacer v0.7 is visually close with multiple `%s%` instances across top and bottom clock lines.
- Current tested lines:
  - Top: `🤖%cpu%🍵%ram%%s%%time%`
  - Bottom: `%weekday%%s%📅%s%%date%%n%🛫%upload_speed%%s%🛬%download_speed%%n%🧮%gpu%🧮%gpu%%s%💽%disk_read%%n%%weather%`
- Known limitation: `%s%` spacer handling does not work inside the weather string.

## 2026-06-15 — Clock Spacer PR readiness pass

- User confirmed Clock Spacer has been working and toggled off/on several times without issues.
- Hardened the initial scan thread: it now has a stop event and is waited during unload with a sent-message pump.
- Documented the weather-string limitation and added a maintainer note that the `%s%` token could be absorbed into Taskbar Clock Customization if desired.
- Bumped Clock Spacer to v1.0 for submission.
- Copied `clock-spacer.wh.cpp` to `windhawk-mods/mods/taskbar-clock-spacer.wh.cpp`, committed as `89f1bc6f` on branch `add-taskbar-clock-spacer`, pushed to fork, and opened PR #4443: https://github.com/ramensoftware/windhawk-mods/pull/4443
- Root README now marks Clock Spacer v1.0 as PR submitted.
