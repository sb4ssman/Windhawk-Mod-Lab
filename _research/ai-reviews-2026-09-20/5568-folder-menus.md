<!-- ai-review sha=ed1d26f6ce98c1b048c7c5a9234a9d1e4a0ccc4d -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

Good round: the 0.7 compat layer, the Desktop-root dedupe by parsing name, the taskbar-DPI menu bitmaps, `StrRetToBufW`, the exception logger, the `g_appPlacement` optional, and the re-entry guard all check out. Four things remain — two in the unload path and the icon flow, one in the new compat code, one in the README.

**1. `Wh_ModUninit` still has a give-up branch, and the pending-Shell-verb path is outside the wait entirely.**

*(a)* The unwind loop is capped at `attempt < 50` (≈5 s), after which the code unhooks, removes the subclass and proceeds "anyway". Unhooking doesn't help with the actual hazard: the taskbar thread's stack still holds `ShowFolderMenu`'s frame (and `TrackPopupMenu`'s return address into it), so when the popup finally closes it returns into the freed image. The comment says "Giving up here is not an option" — the code still gives up. And the cap is reachable in normal use: after `TrackPopupMenu` returns, the tail runs `InvokePidl` → `ShellExecuteExW` *before* `g_menuLoopDepth` is decremented, and an unreachable network target can sit in that call well past 5 s. Drop the cap. Each `RunFromWindowThread` is a `SendMessage` that only completes when the taskbar thread pumps, so the loop already follows the thread's real progress rather than a wall clock:

```cpp
while (g_menuLoopDepth.load() > 0) {
    RunFromWindowThread(hWnd, [](void* parameter) {
        EndMenu();
        SendMessageW(static_cast<HWND>(parameter), WM_CANCELMODE, 0, 0);
    }, hWnd);
    WaitForSingleObject(g_menuIdleEvent, 100);
}
```

The `!menuIdle` branches and the "Retaining the menu idle event" handle leak then go away, and `CloseHandle(g_menuIdleEvent)` becomes unconditional.

*(b)* `InvokePendingShellCommand` runs from `MenuOwnerSubclassProc` *after* `ShowFolderMenu` has released `g_menuLoopDepth` and signalled idle. Your own comment there notes that "a modal Shell verb such as Properties blocks this UI thread" — and Delete's confirmation, Open With, and the Send-To handlers all pump a modal loop on the calling thread. During that loop the stack is `MenuOwnerSubclassProc → InvokePendingShellCommand → InvokeCommand → dialog`, `g_menuLoopDepth` is 0, so uninit doesn't wait; it removes the subclass (which only affects *future* messages), returns, and Windhawk frees the image. When the user closes the dialog, `InvokeCommand` returns into unmapped memory. Bracket the invoke with the same counter/event the menu loop uses so uninit's loop covers it — a tiny RAII scope shared by both sites keeps the `fetch_add`/`ResetEvent`/`fetch_sub`/`SetEvent` pairing in one place:

```cpp
struct MenuPathScope {
    MenuPathScope() { g_menuLoopDepth.fetch_add(1); if (g_menuIdleEvent) ResetEvent(g_menuIdleEvent); }
    ~MenuPathScope() { if (g_menuLoopDepth.fetch_sub(1) == 1 && g_menuIdleEvent) SetEvent(g_menuIdleEvent); }
};
// in MenuOwnerSubclassProc:
    RemoveWindowSubclass(hwnd, MenuOwnerSubclassProc, 1);
    if (g_unloading) { ClearPendingShellCommand(); return 0; }
    { MenuPathScope scope; InvokePendingShellCommand(hwnd); }
    return 0;
```

Unload then waits until the dialog is answered — a self-resolving wait, versus a crash.

**2. Native icons are cached but never applied when the mod is enabled on a running Explorer, or when "Use native Shell icon" is turned on in settings.** Trace either flow:

- `Wh_ModAfterInit` (or `Wh_ModSettingsChanged`) runs `ApplyAllSettings` directly → `BuildFolderButtonGrid` → `NativeFolderIcon` finds nothing in `g_folderIcons` → label fallback. Grid injected.
- `StartRetryThread` → worker: `PrepareFolderIcons()` caches the icon → `ApplyAllSettingsOnWindowThread()` → `ApplyAllSettings`: `liveButtonGrid == g_buttonGrid`, no collision → `rebuild` stays false → `InjectButtonGrid` finds `TaskbarFolderMenuBar` already present and returns `true` without rebuilding (`InjectAfterAppIcons` has the same early return on `g_appPlacement->group`).

So the "one reconciliation even if the initial label-only injection succeeded" the comment in `Wh_ModAfterInit` promises never rebuilds the buttons, and the icon only shows after the next Explorer restart or `StartTaskbar`. It works on Explorer start only because the first direct apply fails there (no taskbar yet). Simplest fix: drop the direct `ApplyAllSettings*` call in both `Wh_ModAfterInit` and `Wh_ModSettingsChanged` and let the worker's first pass (delay 0) do prepare → apply, which orders the cache before the first build and also removes the label→icon flash. Alternatively keep the direct apply but have `PrepareFolderIcons` report whether it added entries and force `rebuild = true` (and bypass the `InjectAfterAppIcons` early return) on the pass that follows.

**3. `LoadLegacyFolders` skips `ExpandEnv`.** `LoadFolders` does `ExpandEnv(Trim(target))`; the legacy reader does `Trim(...)` only. `SHParseDisplayName` doesn't expand `%VAR%`, and `%DESKTOP%` / `%DOWNLOADS%` / `%DOCUMENTS%` are this mod's own tokens — so a 0.7 entry like the README's own `%USERPROFILE%\Downloads` example comes through the compat layer as "(target not found)". One-line fix: `ExpandEnv(Trim(GetStringSetting(L"folders[%d].target", i)))`.

**4. The README is now out of step with 2.0's own behavior** (`taskbar-folder-menus.wh.cpp#L96-L160`):

- The `Layout → Arrangement` table still has unescaped `|` inside code spans (`` `1 | 2 | 3` ``), which GFM splits into cells — this was in the last round and is unchanged. Escape them: `` `1 \| 2 \| 3` ``.
- "Upgrading from 0.7" still says "Old flat keys are not migrated automatically. Re-enter your folders…" — the opposite of what `Behavior.UseLegacySettings` now does. Rewrite it to describe the compat behavior (legacy value used until the 2.0 counterpart is changed; turn the switch off to stop), and add the setting to the settings table.
- Same section: "Defaults remain … unlimited menus" — `MaxMenuItems` now defaults to 150 (the table row above says so).
- `Behavior.ShowHidden` row: "Include hidden and system items" — the setting was reworded to hidden-only.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

* **Dead template code, still present and now larger.** Nothing in the mod references `tbh::RetryLoop`, `tbh::GetMetrics` / `LayoutModelApplies` / `OrientationName`, `ngl::PixelsToDip` / `RowsInHeight` / `AvailableRows` / `AlongAxis` / `ContentAlong` / `TokenIndexWithPrefix` / `Measure` / `Arrange`, `sio::StringSetting` / `LoadString` / `LoadBool` / `LoadChoice` / `Clamp`, `vtw::CollectDescendants` / `FindInnerStackPanel`, or `igc::Acquire`. `RetryLoop` in particular grew by ~60 lines this round to solve "Stop is called from more than one thread" — but the mod still runs its own `StartRetryThread` / `StopRetryThread`, which has exactly the hole the class comment describes: two concurrent `StopRetryThread` calls (uninit on Windhawk's thread, `StartTaskbar` → `StartRetryThread` on the taskbar thread) both take `g_retryLock`, the second sees null handles and returns without waiting, and if that second caller was uninit the worker can outlive `Wh_ModUninit`. Either adopt `RetryLoop` (the fix is already written) or delete it — right now the fix lives only in dead code. The commentary about other mods, other PRs, "this lab" and `verify-template-parity.ps1` is likewise unactionable for a reader of this file.
* **`GetSystemMetricsForDpi` doesn't need dynamic resolution.** `SmallIconMetricForTaskbar` `GetProcAddress`es it "so an older build simply keeps the old behaviour", but two lines later it calls `GetDpiForWindow` directly (same 1607+ baseline), and the mod is Windows 11-only. Just call it.
* **`LoadFolders` still uses raw `Wh_GetStringSetting` + `Wh_FreeStringSetting`** while the variadic `GetStringSetting(name, args...)` you added sits right below it (move it above and use it, as `LoadLegacyFolders` does). Likewise `sio::LoadChoice` exists for `Layout.FillOrder` / `Justify` / `NewItems` but the code still compares with `==`.
* **`PreferCurrentOrLegacyInt` clamps before comparing to the default.** A 2.0 value the user typed outside the range can clamp *onto* the default and silently hand over to the legacy value — e.g. `BorderThickness` = -5 → clamp → -1 == default → 0.7's `borderThickness` wins. Compare the raw read against the default, then clamp.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

* **Desktop-root dedupe relies on adjacency after an unstable sort.** Items are sorted by display name only; with three items sharing a display name (e.g. `report.docx` in both Desktop folders plus `report.pdf`), `std::sort` may interleave them so the two `report.docx` aren't adjacent and survive. Add `fileName` as a secondary key in the comparator (or use `std::stable_sort`) and the pass becomes exact.
* **"Until you change its 2.0 counterpart" isn't quite the rule.** A 2.0 value set explicitly *to its default* (e.g. `ItemWidth` back to 24) still yields to a customized 0.7 value, because the check is `current != currentDefault`. Probably fine given the switch exists, but worth a clause in the `$description` ("…to a value other than its default").

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
