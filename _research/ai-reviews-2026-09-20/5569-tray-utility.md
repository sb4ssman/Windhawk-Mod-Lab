<!-- ai-review sha=b25bf87ae125b3818c401b5c9cb217d8231ecfbe -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

The previous round's findings are addressed: the `Content` semantics are now documented and logged, an unmanaged anchor leases a slot via `AcquireAt` instead of sharing the column (verified on both the `Grid` and `StackPanel` paths, including the shift/unshift of the host markers around `Release`), the 1.x keys are read back as a fallback (the same "kept for compatibility with old settings" pattern the maintainer uses in [taskbar-clock-customization.wh.cpp#L5604](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-clock-customization.wh.cpp#L5604)), the axis-relative/template dead code is gone, and the `RetryLoop` rewrite closes the double-`CloseHandle` race. One regression came in with the `g_stoodDown` changes.

**1. Transient "not ready yet" states now permanently retire the retry loop.** In 1.x the three early exits at `tray-utility-customizer.wh.cpp:3519-3528` (tray below minimum height), `:3549-3553` (no layout items) and `:3670-3674` (no placements) returned `true` without touching the retry, so the bounded retry kept going while the tray finished populating. 2.0 sets `g_stoodDown = true` on all three, and `LayoutIsApplied()` (`:4146`) then tells the retry thread to stop after the very first attempt — which for the `TrayUI::StartTaskbar` path runs immediately, with no delay (`ThreadMain`, `:1660`).

Two of those states are routinely transient at startup:

* `SystemTrayFrameGrid` is part of the SystemTray frame's own XAML, so it is in the visual tree as soon as the frame is constructed, before its first arrange pass — `trayGrid.ActualHeight()` is `0` at that point, `0 < 44` trips the minimum-height check, and the mod stands down for the rest of the session.
* `MainStack` is populated from the system-tray view model after the frame exists. If the first attempt lands before the emoji/touch-keyboard `IconView`s exist (and the chevron is collapsed because there are no hidden icons, which is common), `items` is empty and again the mod stands down for good. Nothing re-triggers it: `WatchHostVisibility` is only reached for tokens that *were* found (`:3120-3129`), and the `LayoutUpdated` drift check only recounts *managed* hosts (`:4036-4050`), so a `MainStack` that fills in later is never observed.

The vertical-taskbar and "every utility switched off" cases are genuinely settled decisions and belong on `g_stoodDown`; these three don't. Suggested shape:

```cpp
    double trayHeight = trayGrid.ActualHeight();
    if (trayHeight <= 0.0) {
        Wh_Log(L"[Apply] Tray not laid out yet");
        return false;  // transient: keep the bounded retry alive
    }
    if (g_settings.minimumTrayHeight > 0 &&
        trayHeight < static_cast<double>(g_settings.minimumTrayHeight)) {
        Wh_Log(L"[Apply] Tray height %.1f is below minimum %d", trayHeight,
               g_settings.minimumTrayHeight);
        g_stoodDown = true;  // settled: a *measured* tray that is too short
        return true;
    }
    ...
    if (items.empty()) {
        Wh_Log(L"[Apply] No layout items found yet");
        return false;  // 1.x behaviour: let the retry come back
    }
```

and the same for the "no placements" exit. Beyond the retry budget, it would be worth remembering the visible-icon count of every *candidate* host at apply time (not just the managed ones) and comparing those in the `LayoutUpdated` check too, so a `MainStack` that populates after a chevron-only apply re-runs the layout instead of waiting for the next settings change or Explorer restart.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users in normal use, so it's your call.

* **`RetryLoop::Start` can be re-entered on the UI thread and orphan a run.** `Start()` → `Stop()` → `StopRun()` pumps sent messages while it waits (`:1676-1684`), and the `Wh_ModSettingsChanged` dispatch (`:4193`) is a sent message to that same thread. If a settings save lands while `OnTaskbarRebuilt` is inside that wait, the nested `Start()` publishes its own run, and when the outer `Start()` resumes it overwrites `run_` at `:1608` without stopping it. The displaced thread keeps running for up to six 1.5 s intervals with nothing able to `Stop()` it; an unload inside that window frees the image under it (it still dereferences `g_unloading` through `run->unloading`). Narrow, but the fix is small — exchange under the mutex and stop whatever was displaced:
  ```cpp
      std::shared_ptr<Run> displaced;
      {
          std::lock_guard<std::mutex> guard(mutex_);
          if (!unloading) displaced = std::exchange(run_, std::move(run));
      }
      if (run) StopRun(run);              // unload began mid-create
      if (displaced) StopRun(displaced);  // a nested Start() got there first
  ```
* **`[[clang::no_destroy]]` on `g_retry` (`:4141`) isn't needed, and the comment explaining it isn't accurate.** `RetryLoop` deliberately has no destructor that waits; the implicit one only releases `run_`, whose `~Run` closes two handles — a plain resource free that is safe at process exit. Per the [global objects page](https://github.com/ramensoftware/windhawk/wiki/Global-objects-and-process-shutdown), suppression is for destructors that block, terminate, or need a specific thread; unneeded suppression is just noise. Drop the attribute (or, if you keep it, fix the comment so it doesn't claim a wait that doesn't exist).
* **Dead code left after the template trim:** `please::Lease::RestoreObject` (`:1154`), `FindDirectTrayHost` (`:2803`) and the static `FindCurrentProcessTaskbarWnd()` wrapper (`:2762`) are never called — the last two are unused `static` functions and will draw `-Wunused-function`. `Metrics::alongDip` / `constrainedDip` now only feed one log line.
* **`Wh_ModSettingsChanged` skips `LoadSettings()` when no taskbar window is found** (`:4188-4192`), so the next `TrayUI::StartTaskbar` rebuild applies the *previous* settings. Without a taskbar window there is no UI-thread layout running, so the torn-read concern doesn't apply on that path — load the settings there and just skip the reapply.
* **`Wh_ModAfterInit` makes exactly one attempt** (`:4180-4184`). If Windhawk attaches to an Explorer that is mid-initialisation (taskbar window exists, tray XAML not yet ready), `ApplyLayout` returns `false` and nothing comes back. Starting the same bounded retry there (`g_retry.Start(RetryAttempt, LayoutIsApplied, g_unloading, 6, 1500, false)`) costs nothing and covers it.
* **`AcquireAt` refuses to lease when a marker with the mod's own name already exists** (`:1845-1847`). The only way one can be there is a previous instance that failed to tear down (e.g. the Uninit dispatch never reached the UI thread); refusing then blocks every later apply until Explorer restarts. Removing the stale marker and proceeding is the more forgiving choice.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

* **The 1.x fallback keeps *every* stored 1.x value, not just customized ones.** Windhawk writes a mod's defaults into settings storage at install, so every 1.x user has `layout = "overflow | emoji | touchKeyboard"`, `position = overflow`, `minimumTrayHeight = 44`, … stored — and `PreferCurrentOrLegacyString`/`Int` (`:2562-2590`) can't tell those from deliberate choices. Two consequences worth stating plainly in the README: upgraders never see `auto` (they keep the 1.x default expression until they edit `Arrangement` or switch the fallback off), and any 2.0 setting that is *returned* to its default silently re-activates the 1.x value — the UI shows `Hidden-icons column` while the mod uses the old `beforeClock`, with only a generic "Using compatible 1.x values" line in the log. If you want to keep the fallback, logging each override as `key <- legacyKey = value` would make that diagnosable, and for the `$options` settings an explicit first choice such as `"default": "Default (hidden-icons column, or your 1.x choice)"` would let a user pick `overflow` and have it win.
* **A deliberate 1.x `minimumTrayHeight = 0`** ("allow any height") is treated as unset by `PreferCurrentOrLegacyInt`'s `legacy == 0` test, so that user silently gets the 2.0 default of 44 after the update.
* **Restore doesn't put a `Grid` tray's children back in their original order.** `CaptureHost` appends the marker (`:3273`) and `ReturnHostToTray` appends the host (`:3300`) on the `Columns` kind, so after a disable/re-enable `NotifyIconStack`/`MainStack` sit *after* the clock and Control Center in the children collection. Column placement hides that visually, but keyboard/UIA traversal follows child order. Inserting the marker at the host's live index on both panel kinds (child order doesn't affect a `Grid`'s layout) and re-inserting at the marker's index on both would make the restore exact and let the `ordered` branch go away.
* **The `LayoutUpdated` drift check is still a 4 Hz full tree walk** (`:4023-4059`) — `CountVisibleIconViews` per managed host, with `winrt::get_class_name` per descendant, throttled to 250 ms but fired by any layout pass in the XamlRoot (clock ticks, animations). Caching the discovered `IconView` list per host, or raising the throttle, would take it off the hot path; noted last round, still your call.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
