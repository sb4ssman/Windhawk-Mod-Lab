<!-- ai-review sha=3cce707bc36db78c4a036b2c5b76c2a4bef13d90 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

All five items from the last round are addressed: the preview class registration is tracked separately from the popup, finished switch threads are reaped on each click, the `Wh_ModUninit` teardown is guarded per step, the Start-placement size is cached, and the settings rename now has a compatibility bridge instead of a silent reset. Reading undeclared 1.7 keys is an accepted technique (m417z's own clock mod does it: [taskbar-clock-customization.wh.cpp#L5604-L5610](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp#L5604-L5610)), so the approach is fine — but the mapping has gaps that defeat it for common 1.7 configurations, and its "default means unset" rule needs a decision.

**1. The 1.7 → 2.0 mapping drops or mis-maps several settings that have direct counterparts.**

Checked against the 1.7 settings block currently on `main`:

* **`labelFormat = "dot"` is not recognized.** 1.7 stored the indicator-symbol mode as `"dot"`; 2.0 calls it `"symbol"`. `PreferCurrentOrLegacyString` (line 2393) hands the raw `"dot"` to the `kLabelFormats` scan, nothing matches, and the user gets `Number`. Every 1.7 user of the ● ○ mode — the very mode this PR set out to improve — comes out of the update with numbers, and the bridge that was supposed to prevent exactly that reports success. One extra row fixes it (harmless for 2.0 users too):
  ```cpp
  static constexpr sio::Choice<VdLabelFormat> kLabelFormats[] = {
      {L"number", VdLabelFormat::Number},
      {L"roman", VdLabelFormat::Roman},
      {L"symbol", VdLabelFormat::Symbol},
      {L"dot", VdLabelFormat::Symbol},   // 1.7's stored value for this mode
      {L"custom", VdLabelFormat::Custom},
  };
  ```
* **`gridVerticalOffset` is mapped to the wrong setting.** In 1.7 it was "Nudge the entire button grid up (negative) or down (positive)" — that is 2.0's `Adjust.OffsetY`, not `Adjust.PadY` (line 2460), which *reserves* symmetric space. As written, a negative nudge (the common "move it up a bit") is clamped away by the `std::max(0, …)`, and a positive one becomes padding that also shrinks the row budget in `AvailableRows`. Should be:
  ```cpp
  g_settings.padY    = std::max(0, Int(L"Adjust.PadY"));
  g_settings.offsetY = PreferCurrentOrLegacyInt(
      L"Adjust.OffsetY", 0, L"gridVerticalOffset", 0, usedLegacy);
  ```
* **The Task View button is lost, and two probed keys are never carried.** `showMasterButton` → `Content.TaskViewButton`, `masterButtonLabel` → `Content.TaskViewLabel`, `masterButtonPosition` → `Content.TaskViewPlacement` (`top`→`above`, `bottom`→`below`), `masterButtonWidth` → `Size.TaskViewSize`, `masterButtonSpacing` → `Size.TaskViewGap`, `shortGroupAlign` → `Layout.Justify` (identical tokens) and `fillOrder` → `Layout.FillOrder` (`rowFirst`→`rows`, `columnFirst`→`columns`) all have one-to-one 2.0 counterparts but aren't read — `masterButtonLabel` and `shortGroupAlign` are even used in `HasLegacySettings()` (line 2296) as proof that a 1.7 config exists, and then ignored. A 1.7 user with the Task View button on finds it gone after the update.
* **The README contradicts the feature.** "Upgrading from 1.x" (line 69) still says "your previous customizations are not migrated; re-apply them once after updating", and the Behavior table (line 266) has no row for the new toggle. Update both to describe what the bridge does and doesn't carry.

**2. Under the bridge, a 2.0 setting at its default is indistinguishable from "unset" — so a carried-over value can never be returned to the default, and the toggle's description doesn't say so.**

`PreferCurrentOrLegacy*` (lines 2317–2342) prefers the 1.7 value whenever the 2.0 value equals its declared default. Concrete trap: a 1.7 user had `position = beforeClock`; after the update they pick **After clock** (the 2.0 default) and press Save — nothing moves, because `current == "afterClock" == currentDefault` hands control back to the legacy key. The same holds for every carried setting (width back to 20, active color back to `accent`, custom labels back to empty…). The `$description` promises the opposite — "keeps working until you change its 2.0 counterpart" — and the only escape (turning **Keep my 1.7 settings** off) isn't mentioned as the way to get a default back. Meanwhile the settings UI shows the 2.0 defaults for everything, so what the user sees and what the bar does disagree for every carried value.

There's a second uncertainty in the same mechanism that I couldn't settle from the headers: what the Windhawk UI does with keys that are no longer declared when the user presses **Save**. If it rewrites the Settings section from the declared keys (which is what would keep stale `key[N]` array entries from lingering), every legacy value disappears at once the first time the user saves *any* 2.0 setting — the opposite of the toggle's promise. Please test this specific sequence: update from a saved 1.7 config, confirm the carried values apply, then change one unrelated 2.0 setting (say `Behavior.PreviewDelay`), save, and check whether position/sizes/colors survive.

Two ways to make the semantics honest, pick one:

* **Bridge until first save.** Treat the first `Wh_ModSettingsChanged` in 2.0 as the hand-over: persist `Wh_SetIntValue(L"legacyHandedOver", 1)` there, and skip the legacy path once it's set. Until then the 1.7 values carry; after the user has looked at the 2.0 settings and saved, what the UI shows is what they get (the README's "re-apply once" already frames it that way). This removes the default-vs-unset ambiguity, no longer depends on undeclared keys surviving a save, and makes the `UseLegacySettings` toggle unnecessary — which is good, because as declared it stays in the UI forever for users who never had 1.7. If the bridge must also survive a Save (in case Windhawk does rewrite the section), snapshot the legacy values into mod storage (`Wh_SetStringValue`/`Wh_SetIntValue`) the first time they're seen and read them from there.
* **Keep the current rule, but say it.** Spell out in the toggle's `$description` and the README that a carried-over setting can only be returned to its default by turning the toggle off, and confirm (per the test above) that legacy values do survive a save. This is the smaller change, but it leaves the UI-shows-default-bar-does-something-else mismatch in place, so I'd lean to the first option.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users in normal operation, so it's your call.

* **The dead template code grew instead of shrinking.** The last round listed the unused template blocks; this round rewrote one of them. `tbh::RetryLoop` (lines 1965–2085, ~120 lines including a new `shared_ptr<Run>` design and a long rationale comment) is still never instantiated — the mod's retry worker is the raw `CreateThread` in `StartRetryThread` (line 5262). Also still unreferenced: the whole `property_lease` namespace (lines 2097–2181, now with a new `RestoreObject`), `ngl::ContentAlong`, `ngl::AvailableRows` + `ngl::PixelsToDip`, `ngl::ResolveArrangement`, the uncached `ngl::Measure`/`ngl::Arrange`, `vtw::CollectDescendants`, `vtw::FindInnerStackPanel`, `sio::LoadString` (both overloads), `GetWindowsAccentBrush` (line 3481), and `VdPositionName` (line 2205). Windhawk mods are single-file; code that only exists to "stay in sync with the template" is review cost with no runtime benefit, and maintaining it (as happened here) compounds that. Trimming to what the mod calls would take a few hundred lines off a 5,400-line file.

* **A settings change that hits the deferred path leaves the mod frozen.** `Wh_ModSettingsChanged` stops the notification thread first (line 5396), and only `ApplyAllSettings()` → `InjectButtonGrid()` restarts it. On the "No live XAML root; deferring reapply" branch (line 5414) — or when the dispatch itself fails — `ApplyAllSettings()` is never reached, and the retry thread that `StartRetryThread()` launches exits on its first pass because `if (g_buttonGrid || g_unloading) break;` (line 5275) sees the *old* grid still in the tree; even if it ran, `InjectButtonGrid` returns early on the existing `VdSwitcherBar` (line 4645). Net effect: the old bar stays with the old settings and stops following desktop switches until an Explorer restart. Rare (it needs `GetTaskbarXamlRoot` to fail while the bar exists), but the fix is small: a `std::atomic<bool> g_reapplyPending` that the retry loop honors (don't `break` on `g_buttonGrid` while pending, and have the attempt do `RemoveButtonGrid(); ApplyAllSettings(); RefreshSecondaryBars();`), and restart the notification thread regardless of the outcome.

* **`tbh::RunFromWindowThread` runs each callback once per concurrently installed hook.** Every caller installs its own `WH_CALLWNDPROC` hook with the same procedure, and each hook instance handles *every* message equal to `g_dispatchMessage` (line 1729). If two threads dispatch at the same time, both hooks are in the chain when either message arrives, so each `Dispatch` is invoked twice. Here the only overlapping pair is the retry thread and the notification thread during the `hideWhenSingle` window (both run `RebuildButtonGrid`/`ApplyAllSettings`, which tolerate a repeat), so it's benign today, but the template will be copied. A `bool ran = false;` in `Dispatch` checked and set inside the hook (`if (!dispatch->ran) { dispatch->ran = true; … }`) closes it — both hooks run sequentially on the UI thread.

* **The `!tornDown` branch does what its own comment says it avoids.** `g_buttonEventStates->clear()` (line 5377) destroys `ButtonEventState` records whose `Grid owner` / `ButtonBase button` members are strong XAML refs, so it *does* call `Release()` on XAML objects from Windhawk's thread — the comment above it ("does not touch XAML") is wrong, and the paragraph below explains why the other XAML owners are deliberately *not* released there. Not the final reference (the elements are still in the tree), so it's unlikely to bite, but pick one rationale and make the code match. While there: `UnregisterClassW(kClass, state.module)` needs no particular thread, so when the popup dispatch fails you can still unregister the preview class from the current thread if `g_previewClassRegistered` is set, and spare the next load the `ERROR_CLASS_ALREADY_EXISTS` path.

* **`static std::atomic<bool> s_reapplied`** (line 5410) exists only because the callback is a plain function pointer; pass a small struct (`HWND hWnd; bool reapplied;`) through the `void*` parameter instead. Similarly, `InvalidateButtonGridSizeEstimate()` at line 5399 runs on Windhawk's thread while the `LayoutUpdated` handler reads the same two globals on the UI thread — it can move into the UI-thread callback next to `RemoveButtonGrid()`.

* **Two string-setting wrappers are now in use side by side**: `sio::StringSetting` (line 1528, used by the `Str` lambda) and `WindhawkUtils::StringSetting` (used by `LegacyString`, line 2291). The utility one is the convention; drop the local copy.

* **Small stuff**: lines 611–616 re-include headers already included above; `<thread>` (line 585) is unused (no `std::thread` anywhere); `-loleaut32` in `@compilerOptions` is unused (no BSTR/VARIANT); `<dwmapi.h>`/`<shobjidl.h>` still sit mid-file (line 3026); the comment at line 5188 says "NEW in v2.1" in a 2.0 mod; the `[Init] VD Switcher v2.0` literal (line 5230) can be `WH_MOD_VERSION`; and the README link `../omnibutton-customizer/archive/vertical-omnibutton-v1.4.wh.cpp` (line 311) still resolves to nothing on windhawk.net.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations about the feature behavior itself.

* **Hover previews probably never appear for buttons on secondary taskbars.** `Schedule` (line 3315) starts from the primary `Shell_TrayWnd` and only switches to a `Shell_SecondaryTrayWnd` if `GetTaskbarXamlRoot(candidate) == root`. That helper goes through the `TaskbandHWND` window property, which as far as I know only the primary taskbar sets, so the match fails, `origin` is the primary taskbar's client origin, `state.anchor` lands on the wrong monitor, and `Show()`'s `PtInRect(&state.anchor, cursor)` guard silently returns. The feature is experimental and off by default, so this is an FYI; if you want it to work there, resolve the owning HWND differently (e.g. `MonitorFromPoint` on the cursor plus the secondary tray on that monitor).

* **Still open from the last round, unchanged**: each hover does a `CoCreateInstance(CLSID_VirtualDesktopManager)`, a full `EnumWindows` with a `GetWindowDesktopId` per window, and up to 48 `DwmRegisterThumbnail` triples synchronously on the taskbar thread (caching the manager and hoisting the registry reads out of `Schedule` are the cheap wins); windows with an empty title are dropped from both the picture and the "%d windows" count; past the 48-window cap the footer under-reports; `DWM_TNP_SOURCECLIENTAREAONLY` is set while `fSourceClientAreaOnly = FALSE`; and timer 2 polls the cursor every 100 ms while the popup is up although `PointerExited`/`PointerCanceled` already cover the exit. All fine to leave as they are if you've weighed them.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
