===== sb4ssman 2026-09-21T17:51:28Z
/ai-review

===== windhawk-reviewer[bot] 2026-09-21T19:00:54Z
<!-- ai-review sha=c48a4f1e12c0878c3d846c0aa806a270dafb5a0e -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

This round takes essentially the whole previous list: `RetryLoop::Start` now publishes by exchange and waits on whatever it displaced, a dispatch that never ran no longer loses the settings change (`SettingsDispatch.loaded` + `LoadSettingsHereAndScheduleRetry`), the rebuild path in `OnLayoutUpdatedImpl` is under `ApplyingScope`, `Dispatch::ran` stops two in-flight hooks from invoking the same callback twice, the items host keeps a 16-DIP hit target when everything is hidden, a DPI or taskbar-height change now triggers a re-apply, the README covers the icon-tweaks overlap and multi-tray scope, `sio::StringSetting` is gone in favour of `WindhawkUtils::StringSetting`, and the PR-number citations are out of the comments. One new item, a side effect of one of those fixes:

1. **`OnLayoutUpdatedImpl` still infers "a slot appeared" from `childCount`, but `ApplyLayout` now deliberately leaves `g_networkPresenter` / `g_volumePresenter` null when the battery sits in slot 0 / 1 — the two disagree, and the outcome is a rebuild on every layout pass.** Lines 3850–3853 set `changed` whenever `!g_networkPresenter && childCount >= 1` or `!g_volumePresenter && childCount >= 2`. With `hasNetwork = n >= 1 && battIdx != 0` / `hasVolume = n >= 2 && battIdx != 1` (3491–3492), the presenter stays null in exactly the case those flags were added for, so every `LayoutUpdated` reports a change, `CleanupAndResetCurrentElements` + `ApplyLayout` run, their invalidations produce the next layout pass, and it never settles — a full teardown and re-apply per frame on Explorer's UI thread (`g_inLayoutUpdated` blocks recursion, not this). The trigger is still hypothetical today (`taskbar-tray-system-icon-tweaks` collapses icons rather than removing them, and current builds keep the battery last), but the guard now turns a "styled twice" cosmetic into a busy loop, which is the failure mode the comment at 3537–3545 describes. Record what `ApplyLayout` saw and compare against that instead of re-deriving from the count:

   ```cpp
   static int g_appliedChildCount = -1;   // set to -1 again in ResetElementRefs

   // ApplyLayout, right after `int n = VisualTreeHelper::GetChildrenCount(sp);`
   g_appliedChildCount = n;

   // OnLayoutUpdatedImpl, replacing the two presenter checks
   if (childCount != g_appliedChildCount)
       changed = true;
   ```

   This keeps the case the old checks were for (apply ran before the children were realized: 0 → 3) and also catches a child *removed* from the items host, which nothing detects today. If you prefer to keep the presenter checks, the minimal form is to store `battIdx` in a global and add `&& g_batteryChildIndex != 0` / `!= 1` to them.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **Stale comment in `omni_retry`** (2301–2302): "Start() itself is not serialized against a concurrent Start(), because both of this mod's callers run on the taskbar's UI thread." `StartRetryThread` is also called from `Wh_ModAfterInit` and from `Wh_ModSettingsChanged` / `LoadSettingsHereAndScheduleRetry` on Windhawk's thread, and the exchange-and-`StopRun(displaced)` block at 2341–2358 exists precisely because two `Start()` calls can overlap. The code is right; the sentence now contradicts it.
- **Dead code carried over from the shared components**, still with no caller: `ngl::AlongAxis` (740) — and with it the whole axis-relative `Size` path (`axisRelative` / `thickness` / `cross`, the axis-relative branches in `MeasureNode` and `ConcreteSize`, and the `inner.axisRelative` handling in `ngl::Compute` and `ComputeOmniNode`; `ContentAlong` returns a plain width × height, so nothing in this mod ever produces an axis-relative size); `ngl::TokenIndexWithPrefix` (979); `ple::Lease::RestoreObject` / `Abandon` / `Count` / `Empty` (1882–1911); `Metrics::alongDip` (2234); and `RetryLoop`'s `forceFirstAttempt` (2317 / 2327 / 2382 / 2402 — never passed `true`). Two comments also still point at `_templates/property-lease.h` (2658) and `_templates/taskbar-host.h` (4028), which don't exist in this repo.
- **`setting.get() ? setting.get() : L""`** (592, 612): `Wh_GetStringSetting` never returns null — it returns `L""` when unset or on error — so `PCWSTR value = setting.get();` is enough.
- **Includes**: `<cwctype>` and `<cstdlib>` left with the old template block, but `iswspace` / `towlower` / `iswxdigit`, `wcstod` / `wcstoul`, and the `wcs*` / `_wcsicmp` / `wcsncpy_s` calls are all still here and now compile through transitive includes. Add `<cwctype>` and `<cwchar>` explicitly.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **The new geometry-change re-apply bypasses the vertical stand-down.** `LayoutModelApplies` is checked only in `ApplyAllSettings`; the rebuild in `OnLayoutUpdatedImpl` (3886–3898, 3956–3964) calls `ApplyLayout` directly. If the taskbar turns vertical while the monitor is attached (the vertical mod enabled mid-session), `constrainedDip` flips from height to width, `changed` fires, and the mod re-arranges into the rotated space instead of standing down — before this round nothing would have triggered a rebuild at that moment. Cheapest fix: when the metrics changed and `!taskbar_metrics::LayoutModelApplies(metrics)`, do `CleanupAndResetCurrentElements(); RevokeLayoutUpdated(); return;` and let the next `TrayUI::StartTaskbar` re-evaluate, rather than setting `changed`. Alternatively route the rebuild through `g_reapplyPending = true; g_applied = ApplyPendingSettings();`, which already carries the check (at the cost of re-walking to `ControlCenterButton`).
- Carried over: network and volume are still positional (slots 0 / 1) while the battery is structural. The new flags stop a double assignment; they don't make slot 0 *be* the network icon if a build reorders. Fine as a documented limitation.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `c48a4f1`.

**Code regions**

- Line 1–11: metadata — `@include explorer.exe`, `@architecture x86-64`, links `ole32 oleaut32 runtimeobject version`.
- Line 13–367: README — arrangement grammar, per-item styling limits, compatibility notes (icon-tweaks, multi-tray, taskbar-on-top, vertical stand-down), known limitations; screenshots hosted on `raw.githubusercontent.com`.
- Line 369–515: settings block — five groups (Content, Layout, Size, Adjust, Surface); all keys are read with the matching API in `LoadSettings`.
- Line 517–552: includes and `using` directives.
- Line 554–617: `omni_settings` — clamped int/bool reads, `$options` table lookup, fixed-buffer string copy via `WindhawkUtils::StringSetting`.
- Line 619–699: `omni_color_tokens` — parses `#RRGGBB` / `#AARRGGBB`, accent tokens (via `UISettings::GetColorValue`) and `transparent` into a `Color` / `SolidColorBrush`; unparseable means "leave native".
- Line 701–1416: `omni_layout` — arrangement expression parser (`|` / `,` / parentheses / `[dx,dy]`, nesting cap 24), memoized measure + arrange, `RowsInHeight`, auto shape / grid expression, missing-token append, `ResolveArrangement`.
- Line 1418–1827: `omni_glyph_surface` — probes an item's template for `InnerTextBlock` / shapes / any TextBlock and its templated-parent style anchor, counts glyph layers to decide which controls apply, applies color / size / font / opacity through a tracking callback, `MeasureNatural`.
- Line 1829–1917: `omni_property_lease` — snapshots each dependency property's prior local value before the first write; `RestoreAll` puts them back newest-first.
- Line 1919–1954: `omni_taskbar_window` — `EnumWindows` filtered to the current PID for `Shell_TrayWnd`; `ResolveTaskbarWnd` validates a cached handle with `IsWindow`.
- Line 1956–2061: `omni_dispatch` — `RunFromWindowThread`: thread-specific `WH_CALLWNDPROC` hook + registered message + `SendMessage`, unhooked after each call; `Dispatch::ran` de-duplicates overlapping hooks.
- Line 2063–2183: `omni_taskbar_xaml` — `taskbarDllHooks` against `taskbar.dll` (loaded with `LOAD_LIBRARY_SEARCH_SYSTEM32`): CTaskBand vftable, `GetTaskbarHost`, `TaskbarHost::FrameHeight`, `_Ref_count_base::_Decref`, and a `TrayUI::StartTaskbar` hook that fires the rebuild callback; `GetTaskbarXamlRoot` walks CTaskBand → TaskbarHost → FrameworkElement (offset read from `FrameHeight`'s prologue, x64 and ARM64).
- Line 2185–2275: `omni_taskbar_metrics` — `GetWindowRect` + `GetDpiForWindow` → DIPs, orientation from aspect, `LayoutModelApplies`.
- Line 2277–2436: `omni_retry` — `RetryLoop`: one `shared_ptr<Run>` (thread + manual-reset stop event) per attempt, mutex held only across the handoff, `StopRun` waits with `MsgWaitForMultipleObjects(QS_SENDMESSAGE)` so a UI-thread caller keeps servicing the retry thread's `SendMessage`; up to 5 attempts, 2 s apart.
- Line 2440–2482: namespace aliases; `omni_tree_walk::FindDescendant`.
- Line 2484–2589: `ModSettings` (fixed buffers, trivially destructible) and `LoadSettings`.
- Line 2610–2672: cached XAML globals under `[[clang::no_destroy]]` (bare for projected types and `Surface`, `std::optional` for the revoker list and lease), `g_retryLoop`, `TrackProperty` / `RestorePropertySnapshots`.
- Line 2674–2977: layout resolution — item sizes (fit-to-content widths, measured percentage cell), `AvailableOmniRows` (also records the applied DPI / height), token normalization, `ResolveOmniLayout` (auto / manual / parse-error fallback / append-missing, logs the resulting expression).
- Line 2979–3162: XAML helpers — `ApplyOffset` (TranslateTransform on `RenderTransform`), battery-slot detection by class-name substring, inner battery panel walk, surface resolve + capability logging, `ApplyAllItemStyles`.
- Line 3164–3218: `ApplyItemsHostFootprint` — sizes the items host (16-DIP floor), zeroes the button's padding, sets `MinWidth`.
- Line 3220–3269: teardown helpers — `ResetElementRefs`, `RevokeLayoutUpdated`, `CleanupAndResetCurrentElements` (restore lease, invalidate, `UpdateLayout`).
- Line 3271–3433: slot helpers — visibility, `Canvas.ZIndex`, `PrepareSlot`, `PrepareIndependentItem`, `LogItemSubtree`, `ReadNaturalOrigin`, `MeasureItemContentWidth`, `ApplyingScope` re-entrancy guard.
- Line 3435–3828: `ApplyLayout` — detects the battery slot, picks network / volume presenters, styles, measures, resolves the layout, positions network / volume, spans the battery presenter across the footprint and translates glyph and percentage into their cells, widens the group if the percentage would clip.
- Line 3830–3996: `LayoutUpdated` monitor — `OnLayoutUpdatedImpl` detects child-set, percentage, geometry (DPI / height), pending re-measure and text changes and rebuilds; otherwise tops up unresolved glyph styling (bounded to 60 passes); `OnLayoutUpdated` wraps it with a re-entrancy guard and revokes on exception.
- Line 3998–4057: thread helpers — exception logger, `RunFromWindowThread` wrapper (message name embeds `WH_MOD_ID`), `GetTaskbarXamlRoot` wrapper, child-lookup helpers.
- Line 4059–4132: `ApplyAllSettings` (find `Shell_TrayWnd`, vertical stand-down, XamlRoot → `ControlCenterButton` → items-host `StackPanel` → `ApplyLayout` + monitor) and `ApplyPendingSettings`.
- Line 4134–4165: `IconView::IconView` hook — registers an auto-revoked `Loaded` handler that runs `ApplyPendingSettings` unless an apply is in flight.
- Line 4167–4241: system-tray module detection (`SystemTray.dll`, else `Taskbar.View.dll` below v2604, else `ExplorerExtensions.dll`), `systemTrayModuleHooks` (one hook, attempted once per module), kernelbase `LoadLibraryExW` hook for late loads.
- Line 4243–4282: `ApplyOnTaskbarWindowThread` (retry body), start / stop retry, `OnTaskbarRebuilt`.
- Line 4284–4473: lifecycle — `Wh_ModInit` (load settings, `taskbar.dll` hooks, tray hooks or kernelbase `LoadLibraryExW` hook), `Wh_ModAfterInit` (late tray hook + start retry), `Wh_ModUninit` (stop and join retry, UI-thread teardown with per-step guards, best-effort fallback), `Wh_ModSettingsChanged` (load + apply on the UI thread, fallback when the dispatch never ran).

**Side effects of interest**

- Line 1929–1942: other reach outside the process: `EnumWindows` at each taskbar lookup — callback filters on `GetCurrentProcessId()`, so only this process's `Shell_TrayWnd` is matched.
- Line 2030–2057: other reach outside the process: `SetWindowsHookExW(WH_CALLWNDPROC, …, threadId)` on each cross-thread dispatch (retry ticks, settings change, unload) — thread-specific, in-process, removed immediately after the `SendMessage` returns.
- Line 2100–2101: other reach outside the process: `LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)` at init — System32-restricted; the reference is never released (the module is Explorer's own and always resident).
- No file, registry, network or IPC access; the accent color is read in-process via `UISettings`, and everything else is XAML property writes on the taskbar's own tree, all leased and restored on unload.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".

