# Add Taskbar Clock Spacer v1.1

Source: https://github.com/ramensoftware/windhawk-mods/pull/4443#issuecomment-6027890634

<!-- ai-review sha=85d9f69d454a9e857e147ead570f20a0252c0524 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

No blocking issues — looks good to merge.

I checked this round's changes against the full file at `85d9f69`:
- The `Text` property-changed callback is now guarded like the two hooks (lines 846–858).
- `LoadSettings()` now runs on the taskbar thread inside the dispatch (lines 1148–1163).
- `Wh_ModUninit` returns early when the tray module was never hooked (lines 1119–1120). That's safe because `g_states` can only be filled from the tray hooks, which exist only after `g_systemTrayModuleHooked` is set.
- The retry loop stops as soon as there's no `Shell_TrayWnd` (lines 1129–1131).
- `@description` reads as suggested.

Every `g_states` access still happens on the taskbar UI thread, all globals are safe at shutdown, and the unload path still unregisters the XAML callbacks on the owning thread. The two notes below are about the new hidden-line check and the warning counter. They concern how the feature behaves, not stability.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **The fallback in `Wh_ModSettingsChanged` can run while clock ticks are live (lines 1152–1162).** `RunFromWindowThread` returns `false` in two cases: when the dispatch never ran, and when the callback ran but threw (`InvokeUiCallback`, lines 264–272).
  - If any `UpdateSpacerLine` in the lambda throws, `ReloadSettings()` runs a second time on the Windhawk thread while the taskbar thread reads `g_settings` and `g_zeroWidthTicks`.
  - The lines after the one that threw aren't refreshed until their next tick.
  - The comment at line 1158 assumes only the first case.

  This only happens on a settings change and both loads write the same values, so it's harmless in practice. Guarding each line, as `ClearSpacerStates` already does, makes `false` mean only "didn't run":
  ```cpp
  for (auto& state : g_states) {
      try {
          UpdateSpacerLine(state);
      } catch (...) {
          Wh_Log(L"Refresh of one clock line failed; continuing");
      }
  }
  ```

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **Hiding a line that is already spaced doesn't hide it (lines 763–767; README lines 133–134).** The new check relies on `sourceCollapsed` to tell "collapsed by this mod" apart from "collapsed by Taskbar Clock Customization". Once this mod has collapsed the block, that distinction is lost:
  - When the user ticks *Hidden* in TCC, its settings change re-applies styles to the existing block through [`ApplyTextBlockStyles`](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/taskbar-clock-customization.wh.cpp#L4715-L4728), which writes `Collapsed` over a block that is already `Collapsed`. Nothing changes and nothing can be observed.
  - `sourceCollapsed` stays `true`, so the fast path keeps showing the generated panel.
  - This is the order most users will follow: `%s%` already in the format, then *Hidden* turned on. In that case the README promise "stays hidden" doesn't hold.

  The same can happen when Explorer starts. This mod hooks the implementation `DateTimeIconContent::OnApplyTemplate`. TCC hooks the `produce<…, IFrameworkElementOverrides>::OnApplyTemplate` ABI thunk that calls it ([TCC line 5730](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/taskbar-clock-customization.wh.cpp#L5730)). So on a fresh template this mod always runs first, nested inside TCC's call to the original. If the text already contains `%s%` at that point, the block is collapsed before TCC applies *Hidden*.

  Turning this mod off and on recovers the hidden line:
  1. The restore clears `Visibility`.
  2. TCC's *Hidden* callback re-collapses the block.
  3. The new state then sees `Collapsed` without having caused it.

  TCC gives no signal that tells its `Collapsed` apart from yours here. The simplest fix is to make the README sentence match what happens: a line hidden *before* this mod spaces it stays hidden; after hiding a line that's already spaced, turn this mod off and on.

- **The "no spare width" warning can still fire spuriously, and its comment misstates the hook order (lines 536–554).** Because of the nesting described above, the order isn't undefined: on a fresh template this mod always reads the width before TCC sets `MaxWidth`. `g_zeroWidthTicks` is one global counter, not one per line. With both time and date lines spaced, each newly templated clock adds two zero-width evaluations, so a second monitor's clock can make the third and log the warning even though every line gets its width on the next tick. A counter field in `SpacerState` would make "three in a row" apply to each line. This is cosmetic: logging is off by default.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `85d9f69`. This mod is new, so everything below is new.

**Code regions**

- Line 1–11: metadata — `@include explorer.exe`, `@architecture x86-64`, links `ole32`/`oleaut32`/`runtimeobject`/`version`.
- Line 13–164: README:
  - the two requirements: TCC installed, and a fixed clock width;
  - "Try the built-in option first" (TCC's *Justified* alignment), and what `%s%` adds over it;
  - the token table and `{spacer}` for the Weather format;
  - setup, troubleshooting, settings, limitations (including the hidden-line sentence at 133–134), how it works, and the relationship to TCC.
  - Screenshot hosted on `raw.githubusercontent.com`.
- Line 166–181: settings block — `maxWidth` and `minSpacerWidth`, both numbers, both read in `LoadSettings`.
- Line 183–256: includes and XAML `using` declarations; `FindChildRecursive` (depth-bounded DFS over the clock's own subtree); `FindCurrentProcessTaskbarWnd` (`EnumWindows` filtered to the current PID, returns `Shell_TrayWnd`).
- Line 258–330: UI-thread dispatch, `RunFromWindowThread`:
  - a `WH_CALLWNDPROC` hook on the taskbar thread, plus a message registered as `"Windhawk_RunFromWindowThread_" WH_MOD_ID` and sent with a synchronous `SendMessageW`;
  - a `ran` flag so concurrent dispatches don't run twice; the hook is removed right after the send.
- Line 336–398: settings (`LoadSettings`, clamped to 0–4000), `CurrentLayoutKey`, and globals:
  - the `g_unloading`, `g_systemTrayModuleHooked` and `g_warnedNoElasticRoom` atomics;
  - the token constants `%s%` and `{spacer}`;
  - `SpacerState`, which holds weak refs and integers, and the `g_states` vector.
- Line 404–554: token search and splitting (`FindNextSpacer`, `SplitOnSpacer`, `SplitLines`), plus helpers:
  - `CopyTextStyle` copies font, color, alignment and line height;
  - `ApplySegmentAlignment` puts the first segment on the left, the last on the right and the rest in the center;
  - `EffectiveLineWidth` takes the mod's `maxWidth`, else TCC's `StackPanel.MaxWidth`, else 0;
  - width pin and cap helpers, and the "no elastic room" log with its consecutive-evaluation counter.
- Line 560–692: `BuildSpacerGrid` / `BuildLineElement` build the Auto/Star column grid, or a plain `TextBlock` for a line without a token. The in-place fast path (`UpdateLineElementText` / `UpdateGeneratedPanelText`) rewrites text, style and width when the shape is unchanged.
- Line 701–811: `CollapseSourceTextBlock` (zero size and `Collapsed`), `RestoreSourceTextBlock` (`ClearValue` on the same six properties, only for a block the mod collapsed), `RemoveGeneratedPanel`, and `UpdateSpacerLine`, the per-line driver:
  - no token → remove the panel and restore the source block;
  - block collapsed by someone else → skip (763–767);
  - otherwise the fast path, or rebuild the vertical `StackPanel` and insert it at the original's index in TCC's shared panel.
- Line 817–882: registration — `SetupSpacerForTextBlock` (dedupe, prune expired entries, record then build, register a guarded `TextProperty` changed callback); `ApplySpacerToDateTimeContent` finds `TimeInnerTextBlock` and `DateInnerTextBlock` and requires a `StackPanel` parent.
- Line 888–1015: symbol hooks and module selection:
  - `DateTimeIconContent::OnApplyTemplate` (implementation, not the ABI thunk): calls the original, QIs `pThis[1]` to `FrameworkElement`, applies.
  - `BadgeIconContent::get_ViewModel` (optional): gated on runtime class `SystemTray.DateTimeIconContent` and `IsLoaded()`.
  - `GetSystemTrayModuleHandle` picks `SystemTray.dll`; else `Taskbar.View.dll` only when its file version is below 2604 (otherwise `nullptr`, so `ExplorerExtensions.dll` isn't picked by mistake); else `ExplorerExtensions.dll`.
  - One `HookSymbols` call, with the array declared `// SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll`. `TryHookSystemTrayModule` uses an `exchange` guard and then `Wh_ApplyHookOperations`.
- Line 1020–1035: `LoadLibraryExW` hook (late-load path) — once a loaded module matches `GetSystemTrayModuleHandle()`, the tray hooks are installed.
- Line 1047–1067: `ClearSpacerStates` — for each entry, guarded separately: unregister the callback (token zeroed), restore the source block, remove the generated panel; then clear the vector.
- Line 1073–1163: lifecycle:
  - `Wh_ModInit` hooks the tray module if it's loaded, else hooks `LoadLibraryExW` in kernelbase via `GetProcAddress`;
  - `Wh_ModAfterInit` retries the tray hook;
  - `Wh_ModUninit` sets `g_unloading`, returns early when never hooked, and otherwise dispatches `ClearSpacerStates` to the taskbar thread, up to 5×100 ms while a window exists;
  - `ReloadSettings` + `Wh_ModSettingsChanged` reload settings and refresh every line on the taskbar thread, falling back to a direct load.

**Side effects of interest**

- None found — the mod hooks two SystemTray symbols (and kernelbase `LoadLibraryExW` until the tray module loads) and edits only its own process's taskbar XAML tree. Its `EnumWindows` is filtered to the current PID, and its `WH_CALLWNDPROC` hook and registered message target only this process's own taskbar thread. It does no file, registry or network access, uses no IPC, and changes no system settings.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
