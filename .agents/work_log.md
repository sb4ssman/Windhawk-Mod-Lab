# Work Log — Windhawk Mod Lab

Lab-level milestones only. Per-mod historical notes live in `.agents/knowledge/`
with mod-prefixed filenames.

## 2026-04-27 — Lab established

- Created Windhawk-Mod-Lab as umbrella repo
- Subtree-merged Windhawk-Vertical-OmniButton history, now retained under `omnibutton-customizer/archive/`
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

## 2026-06-18 — VDS and Taskbar Folder Menus submission

### Virtual Desktop Switcher → v1.6 (new mod ID `taskbar-vd-switcher`, PR #4484)

- Discovered PR #3932 (`taskbar-vd-switcher`) was merged 2026-06-17 at v1.5.
- Decided to resubmit under cleaner ID `taskbar-vd-switcher` / "Virtual Desktop Switcher".
- Left a deprecation request comment on #3932 asking m417z to disable `taskbar-vd-switcher` once #4484 is reviewed.
- v1.6 changes vs merged v1.5:
  - YAML setting names updated to shared rubric canonical names (`gridRows`/`gridColumns`, `activeBackgroundColor`/`inactiveBackgroundColor`, `opacity`, `groupPaddingLeft`/`groupPaddingRight`, `groupOffsetY`) with `AliasedStr`/`AliasedInt` backward-compat lambdas; old names still work
  - "Master button" renamed to "Task View button" throughout YAML `$name` fields and README
  - `contentOffsetX/Y` removed (was a rubric misapplication; not applicable to text buttons)
  - Full settings table added to both in-mod readme and folder README
  - README screenshots expanded with full gallery

### Taskbar Folder Menus → v0.5 (new mod, PR #4485)

- New features added this session: right-click Shell context menu via `IContextMenu`, "Open in Explorer" header at top of each subfolder popup, default hover color `#4488FF`
- `WM_MENURBUTTONDOWN` define added (`#ifndef` guard for Windhawk's Clang environment)
- `contentOffsetX/Y` removed (rubric misapplication)
- `TextBlock.Foreground` bug removed (was preventing hover color inheritance from VSM resource dict)
- Renamed `SYMBOL_HOOK hooks[]` → `taskbarDllHooks[]` (CI validation requirement)
- Screenshots incorporated into both in-mod readme and folder README
- Full settings table in both readmes

### Fork / CI
- Synced fork main with upstream (208 commits)
- Both PR branches had stray `vertical-omnibutton.wh.cpp` (pulled from fork's diverged main); removed in follow-up commits
- Both PRs passed CI after fixes

## 2026-07-16/17 — Notes reorg + VD switcher highlight fix, accent default, multi-monitor toggle

### Lab housekeeping
- `_claude_notes/` retired in favor of `.agents/` (README = durable rules incl. standing directives, working_notes = current state only, work_log, knowledge/, tools/, outputs/). `CLAUDE.md` and new `AGENTS.md` are identical thin pointers. Root README layout updated. `generate_folder_map.py` re-pathed for its new home.

### VD Switcher (all in-tree, v1.7)
- **Highlight bug root-caused and fixed**: lightweight-styling resources (`ButtonBackground` etc.) resolve once at template application; swapping them on a live button does nothing, so the startup-active button kept its baked-in highlight ("both buttons highlighted"). Removed the in-place `UpdateHighlights` path entirely; every VD notification now does a full grid rebuild (`RebuildButtonGrid`). **Live-tested OK.**
- `activeColor` default changed to `"accent"`; empty now means native surface (matching all other color settings). Critical tokens quoted in all setting descriptions.
- **Experimental `multiMonitor` toggle** (issue #4785 follow-up comment by Deen-0x): secondary taskbars discovered from IconView `XamlRoot()`s (GetTaskbarXamlRoot is primary-only), registry of secondary tray grids persists across toggle flips, injection via shared `InsertGridIntoTrayColumns` helper, rebuilt alongside primary on every notification, pruned on monitor disconnect. Tray positions only; may need Explorer restart after enabling. **NOT yet tested** — testing deferred (Explorer restarts break the user's desktop order/workflow).
- GitHub recon: no PR/issue ever asked for multi-monitor except the #4785 comment; issue #4784 is the hover/hit-test report (full-rebuild change may help it — verify during testing); m417z's pending ask on PR #4516 is a changelog update.

## 2026-06-21 — PR status audit and VDS rename

- Audited all open PRs and maintainer comments across ramensoftware/windhawk-mods and m417z/my-windhawk-mods.
- Clock spacer confirmed stalled: PR #4443 (standalone) and PR #68 (integration) both at impasse; no further action planned.
- Discovered PR #4484 (virtual-desktop-switcher) was a mistake: m417z confirmed mod IDs are permanent; the mod is already published as `taskbar-vd-switcher`.
- Renamed local folder and source: `virtual-desktop-switcher/` → `taskbar-vd-switcher/`, `virtual-desktop-switcher.wh.cpp` → `taskbar-vd-switcher.wh.cpp`. Updated `@id`, `@name`, all asset URLs, and all references across CLAUDE.md, README.md, working_notes.md, work_log.md, research docs.
- Closed PR #4484; opened PR #4516 as clean single-file update to existing mod: https://github.com/ramensoftware/windhawk-mods/pull/4516
- Contacted m417z on PR #3859 to propose retiring vertical-omnibutton in favor of omnibutton-customizer (full superset: any grid, per-element nudge, item reorder). Awaiting response.

## 2026-06-15 — Clock Spacer PR readiness pass

- User confirmed Clock Spacer has been working and toggled off/on several times without issues.
- Hardened the initial scan thread: it now has a stop event and is waited during unload with a sent-message pump.
- Documented the weather-string limitation and added a maintainer note that the `%s%` token could be absorbed into Taskbar Clock Customization if desired.
- Bumped Clock Spacer to v1.0 for submission.
- Copied `clock-spacer.wh.cpp` to `windhawk-mods/mods/taskbar-clock-spacer.wh.cpp`, committed as `89f1bc6f` on branch `add-taskbar-clock-spacer`, pushed to fork, and opened PR #4443: https://github.com/ramensoftware/windhawk-mods/pull/4443
- Root README now marks Clock Spacer v1.0 as PR submitted.
