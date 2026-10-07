<!-- ai-review sha=c93f9a87afb849ef59d3d7d3cc276bfdaaac399d -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

No blocking issues. The last round's item is resolved and the new side-taskbar support holds up, so this looks good to merge.

What I checked against the source:

* **The UI thread no longer waits on the icon worker.** `TrayUI::StartTaskbar` and the new edge watcher both call `WakeRetryThread` → `RetryLoop::StartOrWake` (`L2092-L2110`). If a run is live, `StartOrWake` only sets `woken` and signals the wake event. If no run is live, `Launch`'s `Stop()` waits only for a run that is already marked `finished`, whose thread is already exiting. `ThreadMain` sets `finished` under `gate` on both of its exit paths, so a wake is either seen or refused and never lost.
* **Icon extraction can be cancelled, and failures are remembered.** `PrepareFolderIcons` checks `StopRequested()` between targets. `ExtractFolderIcon` runs with no lock held. `g_failedIconTargets` is cleared only in `Wh_ModSettingsChanged`, after the worker has been stopped.
* **Last round's optional items.** `igc::Acquire`, `sio::LoadString`, `g_injectedSlot` and `forceFirstAttempt` are gone. `RowsInHeight` is used, `<cmath>` is explicit, `g_retry` no longer has the unnecessary `no_destroy`, the init failure path closes `g_menuIdleEvent`, and `AppIconLayoutChanged` gates the `LayoutUpdated` walk. `Controls.Primitives.h` is needed for `ButtonBase::Click`, as you said.
* **Edge watcher lifecycle.** Its delegates capture only the address of the global `g_edgeWatch` and hold weak refs. They are revoked on the UI thread in `Wh_ModUninit`, and both callbacks return early while the mod is unloading or settings are being applied.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

* **Race between the icon worker and `LoadSettings` on a settings change.** 2.1 moved `LoadSettings` onto the taskbar thread (`L4410-L4413`), but the worker still reads `g_settings` directly: `PrepareFolderIcons` iterates `g_settings.folders` and passes `entry.target` by reference into the Shell call (`L3574-L3581`), and `FolderIconSize` reads the button size (`L3502`). `Wh_ModSettingsChanged` assumes the worker is stopped, but that isn't guaranteed:
  1. `WakeRetryThread` on the taskbar thread can pass its `g_updatingSettings` check (`L4277`) just before `L4407` sets the flag.
  2. Its `Launch` can then publish a new run after `StopRetryThread` (`L4408`) has already read `run_`, because the publish step re-checks only `unloading` (`L2172`), not the settings flag.
  3. That run's first attempt walks `g_settings.folders` while the dispatched `LoadSettings` reassigns it on the UI thread. The result is a use-after-free in Explorer.

  This needs a taskbar rebuild or an edge change at the exact moment settings are saved, so it is rare. A clean fix is to give the worker its own snapshot: have `LoadSettings` publish the `{target, useDefaultIcon}` list and the icon size under `g_folderIconsMutex`, and have `PrepareFolderIcons` copy them under that lock instead of reading `g_settings`.
* **Leftovers.**
  * `taskbar_metrics::OrientationName` (`L1927`) has no caller.
  * `Metrics::constrainedDip` (`L1838`) is computed at `L1908` but never read.
  * There are still two aliases for the dispatch namespace: `dispatch` (`L1664`, used only in the `StartTaskbar` hook) and `ui_dispatch` (`L2295`). Your reply says the duplicate alias was removed.
  * `GetStringSetting` (`L2416`) still null-checks `value.get()`, while the comment at `L407` correctly says it never returns null.
  * The `AppIconLayoutChanged` comment says "five cheap reads", but the function compares seven values.
  * The `ComputeTree` and `TokenMatcher` comments (`L964-L969`, `L1131-L1135`) describe "a mod that rewrites the tree" and "a mod with aliases", which this mod is neither.
* **`Layout.Arrangement` `$description` (`L239`)** still says "auto fits the taskbar height". On a left or right taskbar it fits the width. The README explains this, but the settings page is where users will read it.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `c93f9a8`. 🟢 added / 🟡 updated / ⚪ unchanged, compared to the currently merged version (0.7). Most component namespaces are new in 2.x but contain a few diff context lines (blank lines and braces), so they show as 🟡.

**Code regions**

* 🟡 Line 1–11: metadata. Targets `explorer.exe` with `x86-64`. Version 0.7 → 2.1, adds `-lshlwapi`.
* 🟡 Line 13–194: README. Screenshots on raw.githubusercontent.com; native icons (background fetch, failed-target memo); after-app-icons placement; a new "Taskbar position" section for native side taskbars; arrangement grammar; settings table; 0.7 upgrade note.
* 🟡 Line 196–331: settings block. Grouped `Placement` / `Content` (`Folders` array of Label / Target / UseDefaultIcon) / `Layout` / `Size` / `Adjust` / `Surface` / `Behavior`. Every key is read by `LoadSettings`.
* 🟡 Line 333–375: includes and XAML `using namespace` directives.
* 🟡 Line 377–426: `folder_menus_settings`. Clamped int/bool reads and the `LoadChoice` token table used for every `$options` dropdown.
* 🟡 Line 428–607: `folder_menus_button_surface`. Parses hex / `accent*` / `transparent` into brushes, adds an optional shine gradient, and applies foreground, background, border, corner and opacity resources to a XAML `Button`.
* 🟡 Line 609–1215: `folder_menus_layout`. A depth-limited parser for the `Arrangement` expression, memoized measure and arrange, `RowsInHeight`, the auto shape, and grid-expression generation (`across` flips rows and columns for side taskbars). Also appends folders the expression didn't name.
* 🟡 Line 1217–1516: `folder_menus_slot_lease`. Classifies `SystemTrayFrameGrid` as a `Grid` (leases a column) or a `StackPanel` (leases a child index). Resolves named tray anchors, inserts or removes a zero-size marker, shifts sibling columns, and releases a stale marker left by a previous instance.
* 🟡 Line 1518–1552: taskbar window discovery. A PID-filtered `EnumWindows` for `Shell_TrayWnd`, plus validation of the cached handle.
* 🟡 Line 1554–1659: `RunFromWindowThread`. Uses a thread-local `WH_CALLWNDPROC` hook and a registered message, with a `ran` flag so concurrent dispatches don't run a callback twice.
* 🟢 Line 1661–1781: `taskbarDllHooks` on `taskbar.dll`, resolved in a single `HookSymbols` call:
  * the `CTaskBand` vftable, `GetTaskbarHost`, `TaskbarHost::FrameHeight` and `_Decref`;
  * a `TrayUI::StartTaskbar` hook that calls the original, then the mod's rebuild callback.

  Reads the `FrameworkElement` offset from `FrameHeight`'s prologue (x64 and ARM64) to reach the taskbar `XamlRoot`.
* 🟡 Line 1783–2034: `folder_menus_taskbar_metrics`, new in 2.1.
  * Reads `RootGrid`'s `DockingStates` visual state to get the docked edge.
  * Uses the window rect to find the orientation and to detect a taskbar rotated by another mod (vertical window, non-side dock).
  * `EdgeWatch` subscribes to `TaskbarFrame.SizeChanged` (filtered to orientation and thickness changes) and to `DockingStates.CurrentStateChanged`. Both call the mod's re-apply callback.
* 🟡 Line 2036–2286: `RetryLoop`. Each run is a worker thread that owns stop and wake events through a shared `Run`.
  * `Start` stops and joins the current run, then replaces it.
  * `StartOrWake` (taskbar thread) wakes a live run, or starts a new one only when none is live.
  * `StopRequested` lets an attempt cancel itself partway through.
  * `StopRun` waits while pumping sent messages.
* 🟢 Line 2288–2298: namespace aliases.
* 🟡 Line 2300–2506: settings structs and `Trim`. `ExpandEnv` expands `%DESKTOP%` / `%DOWNLOADS%` / `%DOCUMENTS%` via `SHGetKnownFolderPath`, then calls `ExpandEnvironmentStringsW`. `LoadFolders` defaults to Desktop + Control Panel. `LoadSettings` reads everything else.
* 🟡 Line 2508–2616: globals.
  * Unload and settings flags.
  * `no_destroy` XAML holders.
  * Menu-path depth counter, idle event and the `MenuPathScope` RAII type.
  * Per-menu PIDL and bitmap state, and the pending Shell verb.
  * `g_side`, `g_rebuildForEdge` and `g_edgeWatch`.
* 🟡 Line 2618–2663: thin wrappers, `FindChildRecursive` and `FindLiveSystemTrayFrameGrid`.
* 🟡 Line 2665–2944: menu content.
  * `ParseShellTarget` (`shell:Desktop`, Control Panel CLSID fallbacks, `SHParseDisplayName`), `BindFolderFromPidl` and `StrRetToBufW`.
  * Per-item icon bitmaps at the taskbar's DPI.
  * `EnumerateShellFolder`: folders first, Desktop-root dedupe by file name, `MaxMenuItems` cap.
* ⚪ Line 2946–3062: menu item insertion with lazy submenus capped by `MaxDepth`, the "Open in Explorer" header, `InvokePidl` (`ShellExecuteExW` "open"), and invoking the queued Shell verb.
* ⚪ Line 3064–3227: Shell context menu.
  * Gets `IContextMenu` from the parent folder and shows it with a nested `TrackPopupMenuEx(TPM_RECURSE)`.
  * Handles outside clicks and defers the chosen verb.
  * Also lazy submenu population and menu hit-testing.
* 🟡 Line 3229–3355: the `WH_MSGFILTER` proc and `MenuOwnerSubclassProc` on `Shell_TrayWnd`. The subclass runs the posted verb inside a `MenuPathScope`, handles `WM_MENURBUTTONUP`, and forwards owner-draw messages to `IContextMenu2/3`.
* 🟡 Line 3357–3474: `ShowFolderMenu`.
  * Re-entry guard, then builds the popup.
  * Aligns the popup away from the taskbar edge (`ABM_GETTASKBARPOS`).
  * Installs the subclass and the filter hook.
  * Runs `TrackPopupMenu` inside a `MenuPathScope`, re-checking `g_unloading` after the scope opens.
  * Tears down and posts the queued verb.
* 🟡 Line 3476–3609: icon pixel cache.
  * `ExtractFolderIcon` runs with no lock held.
  * `CacheFolderIcon` keeps a memo of failed targets; `ForgetFailedFolderIcons` clears it.
  * `PrepareFolderIcons` runs on the worker in an STA and can be cancelled between targets.
  * `NativeFolderIcon` turns cached pixels into a `WriteableBitmap`.
* 🟡 Line 3611–3746: `BuildFolderButtonGrid`. Resolves the arrangement (on side taskbars, `auto` fills across the width), creates the buttons with tooltips and automation names, and on click calls `ShowFolderMenu`, then resets the button's visual state.
* 🟡 Line 3748–3926: after-app-icons placement.
  * Appends the group to `RootGrid` and reserves room with `TaskbarFrameRepeater`'s right margin (bottom margin on side taskbars).
  * Repositions on `LayoutUpdated`, gated by `AppIconLayoutChanged`, and hides the group when it can't fit.
  * Restores the margin on release.
* 🟡 Line 3928–4020: tray grid removal and lookup, Grid-only column-collision detection and repair, `ClearButtonEventState`, and `RemoveButtonGrid`.
* 🟡 Line 4022–4102: `InjectButtonGrid`. Classifies the tray, resolves the anchor (logging the class and named children on failure), then builds, leases and places the grid.
* 🟡 Line 4104–4216: `ApplyAllSettings` and its UI-thread dispatcher.
  * Starts the `EdgeWatch` and reads the docked edge.
  * Stands down on a rotated taskbar.
  * Rebuilds after an edge change.
  * Then either places the group after the app icons or injects it into the tray.
* 🟡 Line 4218–4280: `OnTaskbarEdgeChanged`, the `StartTaskbar` callback (wake only), `g_retry`, and the `StartRetryThread` / `WakeRetryThread` helpers (40 × 1.5 s; each attempt prepares icons, then applies on the UI thread).
* 🟡 Line 4282–4331: `LogUiCallbackFailure`. `Wh_ModInit` loads settings, creates the idle event and installs the symbol hooks, closing the event on failure. `Wh_ModAfterInit` starts the worker.
* 🟡 Line 4333–4404: `Wh_ModUninit`.
  * Joins the worker.
  * Loops `EndMenu` + `WM_CANCELMODE` until the menu path is idle.
  * On the UI thread, removes the filter hook, subclass, edge watch and grid, and resets the XAML holders.
  * Closes the event.
* 🟡 Line 4406–4430: `Wh_ModSettingsChanged`. Stops the worker, removes the grid and reloads settings on the UI thread, clears the failed-icon memo, and restarts the worker.

**Side effects of interest**

* 🟢 Line 1698–1699: other reach outside the process: `LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)` in `Wh_ModInit`. Restricted to System32 and never freed. It's the same call as in 0.7, moved into the component.
* 🟡 Line 1525–1542: other reach outside the process: `EnumWindows` filtered by `GetCurrentProcessId()` to find `Shell_TrayWnd`. Runs on every apply, dispatch and teardown.
* 🟡 Line 1628–1655, 3418–3420: other reach outside the process: thread-local `SetWindowsHookExW` on the taskbar thread.
  * `WH_CALLWNDPROC` for the duration of each `RunFromWindowThread`.
  * `WH_MSGFILTER` while a folder popup is open.

  Both are unhooked on the same path and again in `Wh_ModUninit`.
* ⚪ Line 3391: other reach outside the process: `SetForegroundWindow(Shell_TrayWnd)` before `TrackPopupMenu`. The window is in the mod's own process.
* ⚪ Line 3016–3025, 3033–3062: other reach outside the process: `ShellExecuteExW` "open" on the clicked item, and `IContextMenu::InvokeCommand` for the verb the user picks from the native Shell context menu (which may be Delete, Open with, Properties…). Both run only on an explicit user click.
* 🟡 Line 2705–2727, 2816–2827, 2843–2944, 3508–3541: external file read: Shell namespace parsing, enumeration and icon extraction for the configured targets (`SHParseDisplayName`, `IShellFolder::EnumObjects`, `SHGetFileInfoW`, `IShellItemImageFactory::GetImage`). Read-only. Runs when a menu opens (UI thread) and in the icon worker. A UNC target means SMB traffic.

Nothing else: no registry access, no network beyond those Shell reads, no IPC, no system-setting changes and no file writes. All XAML edits (the tray column or child lease, the `TaskbarFrameRepeater` margin, the `TaskbarFrame` / `DockingStates` subscriptions) are in-process and undone in `Wh_ModUninit`.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".