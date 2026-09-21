<!-- ai-review sha=6a78c667f8d87b33f796e1eddf2dbcf74785a7ec -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

No blocking issues — looks good to merge.

The previous round's item is taken: every step of the `Wh_ModUninit` UI-thread lambda is now individually guarded, so a throw in one revocation can no longer skip the rest, and the fallback comment is honest about what it can't do. Most of the optional list was taken too — `HookSymbols` is attempted at most once per module, the `Loaded` handler is fenced off from an in-flight apply by `ApplyingScope`, `LoadSettings()` now runs on the taskbar thread with its readers, `OnLayoutUpdatedImpl` validates the cached handle, the `Surface` globals carry a note explaining the bare `no_destroy`, and the `RetryLoop` rewrite closes the double-`CloseHandle` hole. What's left is small.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **`RetryLoop::Start` can still orphan a run when two `Start()` calls overlap.** `Stop()` at the top runs outside `mutex_`, and the store at the bottom is unconditional: if `Wh_ModSettingsChanged` (Windhawk thread) and `OnTaskbarRebuilt` (taskbar thread) both pass `Stop()` before either takes the lock, the second `run_ = std::move(run)` silently drops the first run's `shared_ptr`. Its thread keeps running on its own reference, but nothing tracks it any more — a later `Stop()` (including the one in `Wh_ModUninit`) only signals whatever is in `run_`, so the orphan can outlive the unload. It self-limits (it exits on the next tick once `g_applied` is true), so this needs a settings change coinciding with a taskbar rebuild *and* an unload within seconds of it — hence optional. One extra condition under the lock closes it without changing the locking discipline:

  ```cpp
  {
      std::lock_guard<std::mutex> guard(mutex_);
      if (!unloading && !run_) {
          run_ = std::move(run);
          return;
      }
  }
  // Either unload began mid-Start, or another Start() installed a run in
  // the meantime. Wait for this one rather than orphan it.
  StopRun(run);
  ```

- **A failed dispatch in `Wh_ModSettingsChanged` now drops the settings change entirely.** Moving `LoadSettings()` into the UI-thread lambda is the right call for the torn-read concern, but the failure path no longer covers it: if `RunFromWindowThread` fails *before* invoking the lambda (`SetWindowsHookExW` failing, or the window vanishing between `ResolveTaskbarWnd` and `GetWindowThreadProcessId`), nothing loads the new values, `g_reapplyPending` stays false, and `g_applied` may still be true from before — so the trailing `StartRetryThread()` doesn't fire either, and the change is lost until the next Explorer start. Rare, but the previous version handled it. Simplest fix: treat "the lambda never ran" like the `!hWnd` branch — pass a small `{HWND window; bool loaded;}` struct as the parameter, set `loaded = true` right after `LoadSettings()` inside the lambda, and on return, if `!loaded`, do the Windhawk-thread load + `g_reapplyPending = true; g_applied = false; StartRetryThread()` that the `!hWnd` branch already does.

- **`[[clang::no_destroy]]` on `g_retryLoop` isn't needed, and its comment describes a destructor that doesn't exist.** `RetryLoop` has no user-declared destructor; the implicit one releases `run_` — at most a pair of `CloseHandle`s, which is safe at process exit — and a `std::mutex`, whose destructor is a no-op on Windows. Nothing waits on the thread. The "No destructor on purpose … Stop() waits on a thread" comment reads as if one does. Drop the attribute (as was done for `g_settings` last round), or if you want it as insurance against a future destructor, reword the comment so it doesn't claim a wait. See [Global objects and process shutdown](https://github.com/ramensoftware/windhawk/wiki/Global-objects-and-process-shutdown).

- **The `ApplyingScope` gap in `OnLayoutUpdatedImpl`'s rebuild block.** `ApplyLayout` and `ApplyPendingSettings` are covered, but the rebuild path in `OnLayoutUpdatedImpl` calls `CleanupAndResetCurrentElements()` — whose `button.UpdateLayout()` can raise `Loaded` — *before* reaching `ApplyLayout`'s scope. The nested `ApplyPendingSettings` that results is benign (it re-tracks freshly restored values and re-styles surfaces the outer frame is about to reset anyway), but it's the exact re-entrancy the guard exists for. Declaring `ApplyingScope applying;` at the top of the `changed` block covers it.

- **Still open from previous rounds**, briefly: the `forceFirstAttempt` parameter on `RetryLoop::Start` is never passed `true` — `StartRetryThread()` uses the three-argument form — and the rationale comment in `ThreadMain` describes a scenario the callers don't produce (both set `g_applied = false` before starting the loop); `sio::StringSetting` reimplements `WindhawkUtils::StringSetting` (`windhawk_utils.h`, RAII with `get()` and an implicit `PCWSTR`); dead template code carried into the single file — `ngl::AlongAxis`, `ngl::TokenIndexWithPrefix`, `ngl::PixelsToDip` / `ngl::AvailableRows`, `vtw::CollectDescendants`, `vtw::FindInnerStackPanel`, `ple::Lease::Abandon()` / `Count()` / `Empty()`, and now `ple::Lease::RestoreObject`, which arrived this round with no caller; and the source comments in the `RowsInHeight`, `ResolveTaskbarWnd` and taskbar-metrics blocks that cite this repo's pull request numbers, which will mean nothing to a reader after merge.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself. All carried over from previous rounds; the code in question is unchanged.

- Network and volume are still taken as items-host children 0 and 1 while the battery is found structurally, so if `battIdx` ever comes back as 0 or 1 the same element is assigned to both roles and written twice. `hasNetwork = n >= 1 && battIdx != 0` / `hasVolume = n >= 2 && battIdx != 1` costs nothing.
- Hiding all four items makes `layout.total` `{0, 0}` and the OmniButton collapses to roughly zero width, so Quick Settings loses its entry point until the user revisits the mod's settings. A floor on the footprint, or a line in the README, would save someone a confused minute.
- The layout isn't recomputed on a runtime taskbar-height or DPI change — only on settings changes, `TrayUI::StartTaskbar`, or a new `IconView`. Moving a laptop between a 100% and a 150% display mid-session keeps the previous row count until something else triggers a re-apply.
- Only the primary `Shell_TrayWnd` is resolved, so with [taskbar-multi-tray](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-multi-tray.wh.cpp) the secondary copies keep the native arrangement. Probably the right scope; worth a line under "Other taskbar mods".
- `Content.Network` / `Volume` / `Battery` overlap with [taskbar-tray-system-icon-tweaks](https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-tray-system-icon-tweaks.wh.cpp)' `hideNetworkIcon` / `hideVolumeIcon` / `hideBatteryIcon`. Inherent, but a README heads-up would stop users running both sets of toggles against each other.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
