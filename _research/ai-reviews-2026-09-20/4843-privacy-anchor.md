<!-- ai-review sha=1139b8b2649c7264cbfd13d10e4607a87d20cc62 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

All five items from the previous round check out at 1139b8b: the `Loaded` callback is scoped by host stack and `UntrackPrivacyElement` hands a misidentified icon back with its snapshot restored; `MicPrivacyMonitor::Cleanup` unregisters before it detaches, with `m_stopped` and the endpoint pointers under a lock; the bar-removal loop and lease release are guarded, `g_uiHostWnd` is remembered at injection and the unload retry's result is checked; `CancelPendingCameraInit` cancels an in-flight `InitializeAsync` before the join; and `UnwindPublishOnThrow` unwinds the early publish. The settings block and the code still agree in both directions, and the hook arrays name the modules they're resolved against. What's left is one path that sidesteps the gates the rest of the mod enforces. Line numbers refer to `mods/tray-privacy-indicator-anchor.wh.cpp` at 1139b8b.

**1. Windows' own privacy indicators are hidden even when nothing replaces them.** With `Behavior.SuppressNativeIndicators` on (the default), `ApplyPrivacyIndicatorBehavior` collapses every tracked native icon (5769-5773) and the visibility callback re-collapses it (5756-5762) without ever checking that a synthetic icon exists to stand in for it. Three ways that leaves the user with no "mic/camera/location in use" signal at all:

- **The `Loaded` path discards the injection result.** `ApplyStyle` gets this right — `if (!g_syntheticGrid && !InjectSyntheticIcons(root)) return false;` (5858) returns before scanning — but the `IconView` `Loaded` callback calls `InjectSyntheticIcons(root)` (6003), ignores its return, and runs `ApplyPrivacyIndicatorBehavior(fe)` regardless (6006). Every `return false` in `InjectSyntheticIcons` — anchor not found on a new build (5212), unsupported tray panel (5172), placement failure — therefore ends with the native indicator collapsed and no anchor bar. The `g_taskbarRestarted` branch (5998) has the same shape: `ApplyOnTaskbarThread()` may return false or stand down, and the suppression runs anyway.
- **All four `Content` toggles off.** `InjectSyntheticIcons` returns `true` at 5184 without publishing `g_syntheticGrid`, so `ApplyStyle` proceeds to `ScanMainStack` and suppresses the native indicators with nothing on screen in their place.
- **A single `Content` toggle off.** Turning off `Content.Camera` removes the synthetic camera icon, but the native camera glyph (`U+E722`) is still tracked and collapsed when it appears — the README's "turn any of the four icons off individually" silently also turns off Windows' own indicator for that device.

For a mod whose job is to hide the OS privacy indicators, "hidden with no replacement" is the one state that must be unreachable. Suggested shape: register the two callbacks as now, but only write `Visibility`/`IsHitTestVisible` when `g_syntheticGrid` is set *and* the detected type's slot exists (`g_camSlot` etc.), and re-evaluate that in the text callback once the glyph is known — that also removes the need to guess `Location` for an empty glyph. In the `Loaded` path, skip `ApplyPrivacyIndicatorBehavior` when injection failed.

**2. The vertical-taskbar stand-down is bypassed by the same path.** The orientation check lives only in `ApplyOnTaskbarThread` (5890-5901), which returns `true` without injecting. On a vertical taskbar `g_syntheticGrid` is therefore always null, so the first privacy `IconView` that loads takes the `!g_syntheticGrid` branch (5999) and calls `InjectSyntheticIcons(root)` directly — the bar is injected and the column leased, exactly what the stand-down was added to avoid. After an Explorer rebuild it's the other failure: `ApplyOnTaskbarThread` returns at 5900 *before* `g_taskbarRestarted.store(false)` (5907), so `g_taskbarRestarted` stays true, every later `Loaded` re-runs the stand-down, and then suppresses the native indicator with nothing in its place (item 1). Put the `LayoutModelApplies` check inside `InjectSyntheticIcons` (or a shared `CanInject()` that both paths call), and reset `g_taskbarRestarted` before standing down.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users in normal operation, so it's your call.

- **`NonActivatableStack` hosts the language bar, not privacy indicators.** `ScanMainStack` scans only `MainStack`, and [taskbar-tray-system-icon-tweaks](https://github.com/ramensoftware/windhawk-mods/blob/68ea89198430663374733ef5f9ce360c04ef5695/mods/taskbar-tray-system-icon-tweaks.wh.cpp#L1343-L1347) routes `NonActivatableStack` icons to its language-bar handler. With the clause at 3271, an IME icon whose `InnerTextBlock` is still empty at `Loaded` is tracked as a location indicator and collapsed until its text arrives and `UntrackPrivacyElement` gives it back. Dropping the clause makes the hook path match the scan path.
- **`iconView.IsHitTestVisible(true)` at 5775 is the one native write that isn't leased.** It bypasses `TrackProperty`, so it's not restored on unload — and it's unnecessary, since `RestorePropertySnapshots` already puts the property back when suppression is turned off.
- **`tbh::RetryLoop` is still unreferenced.** The rewrite this round (shared `Run`, mutex, `forceFirstAttempt`, the long threading rationale) is ~120 lines of synchronization code the mod never instantiates — it drives its own `g_retryThread`. Either adopt it or drop it. The rest of the unused template surface from the previous round is also still present: `ngl::AlongAxis` / `ContentAlong` / `TokenIndexWithPrefix` / `PixelsToDip` / `AvailableRows` / `Measure` / `Arrange`, `clr::ParseBrush`, `vtw::CollectDescendants` / `FindInnerStackPanel`, the `Anchor` overload of `lease_column::Acquire`, `ple::Lease::Abandon` / `Count` / `Empty`, and `start_placement::Side::Over`.
- **`g_uiHostWnd` is only assigned at the end of `ApplyStyle` (5871).** The `Loaded` path can inject and register every callback without `ApplyStyle` ever completing (5999-6006), leaving it null for unload. Set it wherever injection succeeds (`FindCurrentProcessTaskbarWnd()` is available there), or in `InjectSyntheticIcons` itself.
- **The `catch (...) { Release; throw; }` around `PlaceChild` (5508-5511) is now redundant** — `UnwindPublishOnThrow` runs `RemoveSyntheticIcons`, which releases the lease again and logs a spurious "Privacy column lease was not live". Harmless, but the block can go.
- **Unchanged from the previous round, restated briefly:** `-loleaut32` has no user (no `BSTR`/`VARIANT`/`IDispatch` in the file); `sio::StringSetting` reimplements `WindhawkUtils::StringSetting`; the glow tree (3 halos, up to 3 rings, 9 animations per icon) is built at 5321 even when `Surface.GlowEnabled` is false; `g_taskbarWnd` and `g_settings` are read and written from three threads without synchronization; and `[[clang::no_destroy]] start_placement::Lease g_startLease` (3035) is the bare form on a plain aggregate where `std::optional` with `reset()` would match the neighbouring holders.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **The stand-down doesn't undo UI that's already in place.** If the taskbar becomes vertical while the anchor bar is injected, `ApplyOnTaskbarThread` returns early (5900) and the bar and leased column stay until Explorer rebuilds. Calling `RemoveModUi()` before standing down would make it complete in both directions — and pairs naturally with the fix for item 2.
- **A guessed `Location` type has shared side effects.** `DetectPrivacyType(L"")` is `Location`, so an element tracked while empty owns `g_locActive` until its glyph arrives; `UntrackPrivacyElement` then clears that flag (5664), which can mask a real, still-active location indicator until its next text change. Deferring the type (and the suppression, per item 1) to the first non-empty glyph removes this.
- **`auto` still picks 2 rows on a stock 48 px taskbar.** `AvailablePrivacyRows()` (3352) feeds the whole `Shell_TrayWnd` height into `RowsInHeight` minus only `2 * padY`, so `(48 + 4) / 20 = 2` and four icons default to a 2×2 block — the README describes that as the tall-taskbar case. Measuring the tray host element, or reserving a chrome allowance, would make `auto` match the docs. You've live-tested this, so it may be deliberate.
- **`OpenSettingsForItem` runs `ShellExecuteW` on the taskbar UI thread** from the `Tapped` handler (3662). Launching an `ms-settings:` handler can take a noticeable moment; a worker thread would keep the taskbar responsive.
- **The AppModel watches are expensive relative to what they detect.** The two repository keys are watched with `bWatchSubtree = TRUE` and every notification runs `CheckCopilotInstalled()` over both hives. Any Store app update churns that subtree; a debounce would cut a lot of work for a state that changes rarely.
- **Catalog placement.** No mod duplicates this; the closest are [taskbar-tray-system-icon-tweaks](https://github.com/ramensoftware/windhawk-mods/blob/68ea89198430663374733ef5f9ce360c04ef5695/mods/taskbar-tray-system-icon-tweaks.wh.cpp) (hides the native mic/location glyphs, doesn't reserve space) and your own [tray-utility-customizer](https://github.com/ramensoftware/windhawk-mods/blob/68ea89198430663374733ef5f9ce360c04ef5695/mods/tray-utility-customizer.wh.cpp), which shares the same Placement/Layout/Size/Adjust surface and tray-column lease. Not a blocker, but a user running both ends up with two mods each leasing a tray slot.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
