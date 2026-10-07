Source: https://github.com/ramensoftware/windhawk-mods/pull/4855#issuecomment-6034482944
Date: 2026-10-07T08:54:41Z

<!-- ai-review sha=ff574b0b1065e397aab443aa510764d0c6b6b7d9 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

No blocking issues — looks good to merge.

Last round's findings are all addressed. The `TaskbarFrame` size watcher now reacts only to orientation and thickness (2296–2306), so content-sized Taskbar Styler themes no longer trigger a re-apply when windows open or close. When the layout monitor sees what looks like a rotated taskbar, it now schedules a fresh apply instead of standing down in place (4171–4174). That apply re-reads the edge in `ApplyAllSettings`, and `ApplyPendingSettings` still does the cleanup when the taskbar really is rotated. After that stand-down the monitor stays revoked, so the branch can't loop. The nudge range is now described correctly in the comment and the README, the duplicate alias is gone, and the 1.x history section is down to a short upgrade note.

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `ff574b0`. This mod is new, so everything below is new.

**Code regions**

- Line 1–11: metadata — `@include explorer.exe`, `@architecture x86-64`, links `ole32 oleaut32 runtimeobject version`.
- Line 13–382: README — arrangement grammar and showcase (images on `raw.githubusercontent.com`), nudge and offset limits, per-item styling limits, notes on other tray mods, top/left/right taskbar behavior and the rotated-taskbar stand-down, known limitations.
- Line 384–530: settings block — Content, Layout, Size, Adjust, Surface groups; every key is read with the matching API in `LoadSettings`.
- Line 532–569: includes and `using` directives.
- Line 576–637: `omni_settings` — clamped int/bool reads, `$options` table lookup, fixed-buffer string copy via `WindhawkUtils::StringSetting`.
- Line 639–719: `omni_color_tokens` — parses `#RRGGBB` / `#AARRGGBB`, accent tokens (`UISettings::GetColorValue`) and `transparent` into a `SolidColorBrush`; unparseable means "leave native".
- Line 721–1344: `omni_layout` — arrangement parser (`|` / `,` / parentheses / `[dx,dy]`, nesting capped at 24, nudges clamped to ±100), memoized measure + arrange, `RowsInHeight`, auto shape (with `across` for side taskbars), append-missing policy, `ResolveArrangement`.
- Line 1346–1755: `omni_glyph_surface` — probes an item's template for `InnerTextBlock` / shapes / any TextBlock and its templated-parent anchor, counts glyph layers to decide which controls apply, applies color / size / font / opacity through a tracking callback, `MeasureNatural`.
- Line 1757–1810: `omni_property_lease` — snapshots each dependency property's prior local value before the first write; `RestoreAll` puts them back newest-first.
- Line 1812–1846: `omni_taskbar_window` — `EnumWindows` filtered to the current PID for `Shell_TrayWnd`; `ResolveTaskbarWnd` validates a cached handle.
- Line 1848–1953: `omni_dispatch` — `RunFromWindowThread` via a thread-specific `WH_CALLWNDPROC` hook + registered message + `SendMessage`; `Dispatch::ran` de-duplicates overlapping hooks.
- Line 1955–2075: `omni_taskbar_xaml` — `taskbarDllHooks` against `taskbar.dll` (loaded with `LOAD_LIBRARY_SEARCH_SYSTEM32`), `TrayUI::StartTaskbar` hook firing the rebuild callback, `GetTaskbarXamlRoot` (FrameworkElement offset decoded from `FrameHeight`'s prologue, x64 and ARM64).
- Line 2077–2328: `omni_taskbar_metrics` — window rect → DIPs, `ReadDockedEdge` from `RootGrid`'s `DockingStates`, rotated detection (vertical window without a Left/Right dock), `CanArrange` / `RunsDownSide`, `EdgeWatch` (`TaskbarFrame.SizeChanged` filtered to orientation and thickness, plus `DockingStates.CurrentStateChanged`, both calling back into the mod).
- Line 2330–2566: `omni_retry` — `RetryLoop`: `Start` / `StartOrWake` / `Stop`, one shared `Run` per thread with stop and wake events and a `finished` gate, `StopRun` waits with `MsgWaitForMultipleObjects(QS_SENDMESSAGE)`.
- Line 2568–2612: namespace aliases; `omni_tree_walk::FindDescendant`.
- Line 2614–2719: `ModSettings` (fixed buffers, trivially destructible) and `LoadSettings`.
- Line 2740–2800: cached XAML globals under `[[clang::no_destroy]]` (bare for projected types and `Surface`, `std::optional` for the revoker list and lease), `g_retryLoop`, `TrackProperty` / `RestorePropertySnapshots`.
- Line 2802–3138: layout resolution — token → item mapping (`wifi` alias), item sizes (fit-to-content, measured percentage cell), docked edge / side state, `EdgeWatch` global, `AvailableOmniRows` (rows, or columns across a side taskbar), normalization (logs unknown names), `ResolveOmniLayout`.
- Line 3140–3323: XAML helpers — `ApplyOffset` (TranslateTransform), battery-slot detection by class-name substring, inner battery panel walk, surface resolve + capability logging, `ApplyAllItemStyles`.
- Line 3325–3398: `ApplyItemsHostFootprint` — sizes the StackPanel host (16-DIP floor) or raises the WrapGrid's `MinHeight`, zeroes the button's padding, sets `MinWidth` (and `MinHeight` on a side taskbar).
- Line 3400–3450: teardown helpers — `ResetElementRefs`, `RevokeLayoutUpdated`, `CleanupAndResetCurrentElements`.
- Line 3452–3594: slot helpers — visibility, `Canvas.ZIndex`, `PrepareSlot`, `PrepareIndependentItem`, subtree log, `ReadNaturalOrigin`, `PositionSlot`, `ReadSlotOrigins`.
- Line 3596–3681: side-taskbar layout — clears each item's transform, `UpdateLayout`, then centers each item's drawn element on its arranged cell inside Windows' own `WrapGrid`.
- Line 3683–4127: `MeasureItemContentWidth`, `ApplyingScope` guard, `ApplyLayout` (battery slot, presenters, styling, measuring, layout resolution, then the side path or the bottom/top path that positions network / volume and spans the battery presenter).
- Line 4129–4328: `LayoutUpdated` monitor — a detached host, an orientation change or an apparently rotated taskbar schedules a fresh apply; child-count / battery / geometry / percentage changes rebuild in place; bounded glyph top-up; re-entrancy guard.
- Line 4330–4389: thread and tree helpers — exception logger, `RunFromWindowThread` wrapper (message name embeds `WH_MOD_ID`), child lookups.
- Line 4391–4489: `FindOmniItemsHost`, `ApplyAllSettings` (starts the edge watch, reads the docked edge, rotated stand-down, `ControlCenterButton` → items host → `ApplyLayout` + monitor), `ApplyPendingSettings`.
- Line 4491–4522: `IconView::IconView` hook — auto-revoked `Loaded` handler that runs `ApplyPendingSettings` unless an apply is in flight.
- Line 4524–4598: tray module detection (`SystemTray.dll`, `Taskbar.View.dll` below v2604, `ExplorerExtensions.dll`), `systemTrayModuleHooks` (attempted once per module), kernelbase `LoadLibraryExW` hook for a late load.
- Line 4600–4654: retry body, start / stop, `OnTaskbarRebuilt` and `OnTaskbarEdgeChanged` (both wake the retry via `StartOrWake`), `HookTaskbarDllSymbols`.
- Line 4656–4851: lifecycle — `Wh_ModInit` (settings, `taskbar.dll` hooks, tray hook or `LoadLibraryExW` hook), `Wh_ModAfterInit` (late tray hook, start retry), `Wh_ModUninit` (stop and join the retry, UI-thread teardown with per-step guards incl. `StopEdgeWatch`, best-effort fallback), `Wh_ModSettingsChanged` (load + apply on the UI thread, fallback when the dispatch never ran).

**Side effects of interest**

- Line 1819–1836: other reach outside the process: `EnumWindows` at each taskbar lookup — the callback filters on `GetCurrentProcessId()`, so only this Explorer's `Shell_TrayWnd` is matched.
- Line 1911–1951: other reach outside the process: `SetWindowsHookExW(WH_CALLWNDPROC, …, threadId)` on each cross-thread dispatch (retry attempts, settings change, unload) — thread-specific to Explorer's taskbar thread, removed as soon as the `SendMessage` returns.
- Line 1992–1993: other reach outside the process: `LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)` at init — restricted to System32; the reference is never released (Explorer's own, always-resident module).
- No file, registry, network or IPC access; the accent color is read in-process through `UISettings`, and every XAML write is leased and restored on unload.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
