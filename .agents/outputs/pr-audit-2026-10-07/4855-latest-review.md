# Add OmniButton Customizer v2.1

Source: https://github.com/ramensoftware/windhawk-mods/pull/4855#issuecomment-6027835630

<!-- ai-review sha=f2516de9ebe8442ffaea07e29966909d8579905c -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

The previous round's item is fixed as suggested. `ApplyLayout` records the child count it saw (3755), `ResetElementRefs` sets it back to -1 (3419), and the monitor compares against it (4207). The extra -1 guard is a good catch. The optional items were all taken too, and the monitor now stands down when the taskbar is rotated. Native top/left/right support is new in 2.1, and the one new item comes from it:

1. **The edge watcher re-applies the whole arrangement whenever `TaskbarFrame` changes *length*, though only its thickness and orientation matter.** The `SizeChanged` handler (2310–2320) calls `onChange` when either `Width` or `Height` moves by 0.5 DIP. `OnTaskbarEdgeChanged` (4661–4667) then clears `g_applied`, sets `g_reapplyPending` and wakes the retry loop. The next dispatch runs `CleanupAndResetCurrentElements` and a full `ApplyAllSettings`: tree walk, probes, measures and `UpdateLayout`. On the stock taskbar the frame always fills the window, so this fires only on real moves. But several of Windows 11 Taskbar Styler's built-in themes make the frame content-sized: the OS26 Liquid Glass MacDock variants, FrostyGlass, FrostedAcrylic and Minecraft Hotbar all set `Taskbar.TaskbarFrame` to `Width=Auto` ([MacDock](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/windows-11-taskbar-styler.wh.cpp#L6885-L6887), [FrostyGlass](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/windows-11-taskbar-styler.wh.cpp#L8716-L8718)). With those themes the frame's width changes every time a window opens or closes, so the OmniButton is torn down and re-arranged on Explorer's UI thread each time. The README says Taskbar Styler is compatible, so this is a supported setup. Any other mod that resizes the frame along the bar triggers it the same way. And because a re-apply resizes the tray itself (restore, then re-arrange), it could keep re-triggering in any layout where the frame's width also follows the tray.

   The comment at 2259–2261 already says what this signal is for ("between a horizontal and a side edge and whenever the thickness does"), so filter on exactly that:

   ```cpp
   struct EdgeWatch {
       // ... replaces width / height
       bool side = false;
       double thickness = 0.0;
   };

   // StartEdgeWatch
   watch.side = frame.ActualHeight() > frame.ActualWidth();
   watch.thickness = watch.side ? frame.ActualWidth() : frame.ActualHeight();
   watch.token = frame.SizeChanged(
       [target](winrt::Windows::Foundation::IInspectable const&,
                winrt::Windows::UI::Xaml::SizeChangedEventArgs const& args) {
           auto size = args.NewSize();
           bool side = size.Height > size.Width;
           double thickness = side ? size.Width : size.Height;
           // Only the length along the bar changed (e.g. task buttons coming
           // and going on a content-sized frame): nothing to re-arrange.
           if (side == target->side &&
               std::abs(thickness - target->thickness) < 0.5)
               return;
           target->side = side;
           target->thickness = thickness;
           if (target->onChange) target->onChange();
       });
   ```

   The `DockingStates` subscription still covers bottom ↔ top and left ↔ right, and the `LayoutUpdated` monitor still catches a replaced items host. So nothing the watcher is meant to detect is lost.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **Nudge range** (928–931): the comment says expression nudges are kept "within the same user-facing range as Adjust.OffsetX/Y", but the clamp is ±100 while `Adjust.OffsetX/Y` are clamped to ±40 (2713–2714). Either clamp to ±40 or fix the comment, and mention the limit in the README's *Nudging* paragraph, since it is currently undocumented.
- **Duplicate alias**: `namespace dispatch = omni_dispatch;` is declared twice (1975 and 2590). It's legal, but one of them can go.
- **README "Why this starts at 2.0"** (117–137): this is release history for people who installed 1.x by hand from this PR. Anyone installing from the catalog only ever sees 2.x, so this could shrink to a sentence or move to the PR description.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **The monitor's rotated stand-down reads a cached edge.** `OnLayoutUpdatedImpl` (4170–4171) uses `g_dockedEdge`, which is refreshed only in `ApplyAllSettings` (4454). During a native bottom → left move, a layout pass can see the window already taller than wide before the edge watcher has fired. The stale `Bottom` then makes `rotated` true, and the monitor restores the native button and logs "being rotated by another mod" (4186–4195). The watcher's later event re-applies, so it recovers, but the log line is misleading. A simpler branch that corrects itself: when `metrics.rotated`, call `OnTaskbarEdgeChanged()` and return. `ApplyPendingSettings` already does the cleanup, and `ApplyAllSettings` re-reads the edge and stands down if the taskbar really is rotated. The monitor stays revoked after that stand-down, so it can't loop.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `f2516de`. This mod is new, so everything below is new.

**Code regions**

- Line 1–11: metadata — `@include explorer.exe`, `@architecture x86-64`, links `ole32 oleaut32 runtimeobject version`.
- Line 13–399: README — arrangement grammar and showcase (images on `raw.githubusercontent.com`), per-item styling limits, notes on other tray mods, top/left/right taskbar behavior and the rotated-taskbar stand-down, known limitations.
- Line 401–547: settings block — Content, Layout, Size, Adjust, Surface groups; every key is read with the matching API in `LoadSettings`.
- Line 549–586: includes and `using` directives.
- Line 593–654: `omni_settings` — clamped int/bool reads, `$options` table lookup, fixed-buffer string copy via `WindhawkUtils::StringSetting`.
- Line 656–736: `omni_color_tokens` — parses `#RRGGBB` / `#AARRGGBB`, accent tokens (`UISettings::GetColorValue`) and `transparent` into a `SolidColorBrush`; unparseable means "leave native".
- Line 738–1361: `omni_layout` — arrangement parser (`|` / `,` / parentheses / `[dx,dy]`, nesting capped at 24, nudges clamped to ±100), memoized measure + arrange, `RowsInHeight`, auto shape (with `across` for side taskbars), append-missing policy, `ResolveArrangement`.
- Line 1363–1772: `omni_glyph_surface` — probes an item's template for `InnerTextBlock` / shapes / any TextBlock and its templated-parent anchor, counts glyph layers to decide which controls apply, applies color / size / font / opacity through a tracking callback, `MeasureNatural`.
- Line 1774–1827: `omni_property_lease` — snapshots each dependency property's prior local value before the first write; `RestoreAll` puts them back newest-first.
- Line 1829–1863: `omni_taskbar_window` — `EnumWindows` filtered to the current PID for `Shell_TrayWnd`; `ResolveTaskbarWnd` validates a cached handle.
- Line 1865–1970: `omni_dispatch` — `RunFromWindowThread` via a thread-specific `WH_CALLWNDPROC` hook + registered message + `SendMessage`; `Dispatch::ran` de-duplicates overlapping hooks.
- Line 1972–2092: `omni_taskbar_xaml` — `taskbarDllHooks` against `taskbar.dll` (loaded with `LOAD_LIBRARY_SEARCH_SYSTEM32`), `TrayUI::StartTaskbar` hook firing the rebuild callback, `GetTaskbarXamlRoot` (FrameworkElement offset decoded from `FrameHeight`'s prologue, x64 and ARM64).
- Line 2094–2341: `omni_taskbar_metrics` — window rect → DIPs, `ReadDockedEdge` from `RootGrid`'s `DockingStates`, rotated detection (vertical window without a Left/Right dock), `CanArrange` / `RunsDownSide`, `EdgeWatch` (`TaskbarFrame.SizeChanged` + `DockingStates.CurrentStateChanged`, both calling back into the mod).
- Line 2343–2579: `omni_retry` — `RetryLoop`: `Start` / `StartOrWake` / `Stop`, one shared `Run` per thread with stop and wake events and a `finished` gate, `StopRun` waits with `MsgWaitForMultipleObjects(QS_SENDMESSAGE)`.
- Line 2581–2625: namespace aliases; `omni_tree_walk::FindDescendant`.
- Line 2627–2732: `ModSettings` (fixed buffers, trivially destructible) and `LoadSettings`.
- Line 2753–2813: cached XAML globals under `[[clang::no_destroy]]` (bare for projected types and `Surface`, `std::optional` for the revoker list and lease), `g_retryLoop`, `TrackProperty` / `RestorePropertySnapshots`.
- Line 2815–3151: layout resolution — token → item mapping (`wifi` alias), item sizes (fit-to-content, measured percentage cell), docked edge / side state, `EdgeWatch` global, `AvailableOmniRows` (rows, or columns across a side taskbar), normalization (logs unknown names), `ResolveOmniLayout`.
- Line 3153–3336: XAML helpers — `ApplyOffset` (TranslateTransform), battery-slot detection by class-name substring, inner battery panel walk, surface resolve + capability logging, `ApplyAllItemStyles`.
- Line 3338–3411: `ApplyItemsHostFootprint` — sizes the StackPanel host (16-DIP floor) or raises the WrapGrid's `MinHeight`, zeroes the button's padding, sets `MinWidth` (and `MinHeight` on a side taskbar).
- Line 3413–3463: teardown helpers — `ResetElementRefs`, `RevokeLayoutUpdated`, `CleanupAndResetCurrentElements`.
- Line 3465–3607: slot helpers — visibility, `Canvas.ZIndex`, `PrepareSlot`, `PrepareIndependentItem`, subtree log, `ReadNaturalOrigin`, `PositionSlot`, `ReadSlotOrigins`.
- Line 3609–3694: side-taskbar layout — clears each item's transform, `UpdateLayout`, then centers each item's drawn element on its arranged cell inside Windows' own `WrapGrid`.
- Line 3696–4140: `MeasureItemContentWidth`, `ApplyingScope` guard, `ApplyLayout` (battery slot, presenters, styling, measuring, layout resolution, then the side path or the bottom/top path that positions network / volume and spans the battery presenter).
- Line 4142–4349: `LayoutUpdated` monitor — detached-host / orientation change → scheduled re-apply, rotated stand-down, child-count / battery / geometry / percentage changes → rebuild, bounded glyph top-up; re-entrancy guard.
- Line 4351–4410: thread and tree helpers — exception logger, `RunFromWindowThread` wrapper (message name embeds `WH_MOD_ID`), child lookups.
- Line 4412–4510: `FindOmniItemsHost`, `ApplyAllSettings` (starts the edge watch, reads the docked edge, rotated stand-down, `ControlCenterButton` → items host → `ApplyLayout` + monitor), `ApplyPendingSettings`.
- Line 4512–4543: `IconView::IconView` hook — auto-revoked `Loaded` handler that runs `ApplyPendingSettings` unless an apply is in flight.
- Line 4545–4619: tray module detection (`SystemTray.dll`, `Taskbar.View.dll` below v2604, `ExplorerExtensions.dll`), `systemTrayModuleHooks` (attempted once per module), kernelbase `LoadLibraryExW` hook for a late load.
- Line 4621–4675: retry body, start / stop, `OnTaskbarRebuilt` and `OnTaskbarEdgeChanged` (both wake the retry via `StartOrWake`), `HookTaskbarDllSymbols`.
- Line 4677–4872: lifecycle — `Wh_ModInit` (settings, `taskbar.dll` hooks, tray hook or `LoadLibraryExW` hook), `Wh_ModAfterInit` (late tray hook, start retry), `Wh_ModUninit` (stop and join the retry, UI-thread teardown with per-step guards incl. `StopEdgeWatch`, best-effort fallback), `Wh_ModSettingsChanged` (load + apply on the UI thread, fallback when the dispatch never ran).

**Side effects of interest**

- Line 1836–1853: other reach outside the process: `EnumWindows` at each taskbar lookup — the callback filters on `GetCurrentProcessId()`, so only this Explorer's `Shell_TrayWnd` is matched.
- Line 1928–1968: other reach outside the process: `SetWindowsHookExW(WH_CALLWNDPROC, …, threadId)` on each cross-thread dispatch (retry attempts, settings change, unload) — thread-specific to Explorer's taskbar thread, removed as soon as the `SendMessage` returns.
- Line 2009–2010: other reach outside the process: `LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)` at init — restricted to System32; the reference is never released (Explorer's own, always-resident module).
- No file, registry, network or IPC access; the accent color is read in-process through `UISettings`, and every XAML write is leased and restored on unload.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
