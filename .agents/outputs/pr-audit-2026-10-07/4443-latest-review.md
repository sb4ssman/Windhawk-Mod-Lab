Source: https://github.com/ramensoftware/windhawk-mods/pull/4443#issuecomment-6034519848
Date: 2026-10-07T08:56:29Z

<!-- ai-review sha=cf47511f210c191b86a8911ca9264822ee0802f6 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

No blocking issues — looks good to merge.

All of last round's notes are addressed in `cf47511`:
- **Settings refresh.** Each line refresh in `Wh_ModSettingsChanged` is now guarded on its own (lines 1161–1168), so a throw no longer makes the dispatch report failure. Settings are loaded on the Windhawk thread only when there is no taskbar window at all (lines 1171–1174). If a window exists but the dispatch fails, the old settings are kept and a log line asks the user to save again (lines 1175–1179). Nothing writes `g_settings` off the UI thread while ticks can read it.
- **Warning counter.** The zero-width counter is now a `SpacerState` field (line 397). It resets when the line has width (line 549), has no token (line 761), or is hidden (line 772), and on a settings change (line 1162). It stops counting at the threshold (lines 551–552), so other lines and monitors no longer add to it. The comment at lines 541–544 now gives the actual hook nesting.
- **Hidden lines.** The README Limitations entry (lines 133–138) now matches the actual behavior: a line hidden before it's spaced stays hidden; after hiding an already-spaced line, or after Explorer starts, toggle this mod.

The rest of the file is unchanged since last round. I re-checked it, and the earlier conclusions still hold:
- All `g_states` access happens on the taskbar UI thread.
- The globals are safe at process shutdown: `SpacerState` holds only weak refs and integers, and nothing uses `no_destroy`.
- Unload unregisters the XAML callbacks on their owning thread before the image goes away.
- Each module gets exactly one `HookSymbols` call, and the array's module comment matches the modules it's resolved against.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **The PR description no longer matches the code.** It still says "Namespace-scope XAML state is `[[clang::no_destroy]]`-guarded", but the mod deliberately uses no `no_destroy` (lines 399–403). It also mentions a "Line width override" setting, which doesn't exist; the settings are `maxWidth` and `minSpacerWidth`. The maintainer reads that text when picking up the PR, so it's worth updating before `/ready-for-reviewer`.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **Over-long generated rows are cut off, while the native block shows an ellipsis (lines 459–474, 530–532).** With *Max width* set, Taskbar Clock Customization gives the clock text blocks `TextTrimming::CharacterEllipsis` ([TCC line 4735–4737](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/taskbar-clock-customization.wh.cpp#L4735-L4737)). `CopyTextStyle` doesn't copy `TextTrimming`. So a too-long unspaced row (the weather line, for example) is simply cut off at the edge, while the same text in TCC's native block ends with "…". That contradicts the comment at 530–532 ("clips … exactly like the native text block"). Adding `dst.TextTrimming(src.TextTrimming());` to `CopyTextStyle` makes plain rows match. Grid segments sit in `Auto` columns, so trimming has no effect on them.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `cf47511`. This mod is new, so everything below is new.

**Code regions**

- Line 1–11: metadata — `@include explorer.exe`, `@architecture x86-64`, links `ole32`/`oleaut32`/`runtimeobject`/`version`.
- Line 13–168: README:
  - two requirements: Taskbar Clock Customization (TCC) installed, and a fixed clock width;
  - "Try the built-in option first", covering TCC's *Justified* alignment and what `%s%` adds;
  - the token table and `{spacer}` for the Weather format;
  - setup, troubleshooting, settings, limitations (the hidden-line workaround is at 133–138), how it works, and the relationship to TCC.
  - The screenshot is on `raw.githubusercontent.com`.
- Line 170–185: settings block — `maxWidth` and `minSpacerWidth`, both numbers, both read in `LoadSettings`.
- Line 187–260: includes and XAML `using` declarations, then two helpers:
  - `FindChildRecursive`: a depth-limited search of the clock's own subtree;
  - `FindCurrentProcessTaskbarWnd`: `EnumWindows` filtered to the current PID, returning `Shell_TrayWnd`.
- Line 262–334: UI-thread dispatch (`RunFromWindowThread`). It installs a `WH_CALLWNDPROC` hook on the taskbar thread, registers a message named `"Windhawk_RunFromWindowThread_" WH_MOD_ID`, and sends it synchronously with `SendMessageW`. A `ran` flag stops a callback from running twice when two dispatches overlap. The hook is removed right after the send.
- Line 340–403: settings and globals:
  - `LoadSettings` (values clamped to 0–4000) and `CurrentLayoutKey`;
  - the atomics `g_unloading`, `g_systemTrayModuleHooked` and `g_warnedNoElasticRoom`;
  - the token constants `%s%` and `{spacer}`;
  - `SpacerState` (weak refs, token, collapse flag, per-line zero-width counter) and the `g_states` vector.
- Line 409–560: text handling and layout helpers:
  - splitting text into lines and segments (`FindNextSpacer`, `SplitOnSpacer`, `SplitLines`);
  - `CopyTextStyle` (font, color, alignment, line height);
  - `ApplySegmentAlignment` (first segment left, last right, middle ones centered);
  - `EffectiveLineWidth`: this mod's `maxWidth`, else TCC's `StackPanel.MaxWidth`, else 0;
  - helpers that pin the panel width and cap row widths;
  - `WarnIfNoElasticRoom`: a one-time log after three zero-width evaluations of the same line.
- Line 567–698: `BuildSpacerGrid` / `BuildLineElement` build a grid of `Auto` text columns and `Star` gap columns, or a plain `TextBlock` for a line without a token. `UpdateLineElementText` / `UpdateGeneratedPanelText` are the fast path: when the shape is unchanged, they rewrite text, style and width in place.
- Line 707–819: source-block handling and the per-line driver:
  - `CollapseSourceTextBlock` sets zero size and `Collapsed`;
  - `RestoreSourceTextBlock` clears the same six properties, but only on a block this mod collapsed;
  - `RemoveGeneratedPanel`;
  - `UpdateSpacerLine`: with no token, it removes the panel and restores the block. If someone else collapsed the block, it skips the line (770–775). Otherwise it takes the fast path, or rebuilds a vertical `StackPanel` and inserts it at the original block's position in TCC's shared panel.
- Line 825–890: registration:
  - `SetupSpacerForTextBlock` skips duplicates, drops expired entries, records the state, builds the panel, then registers a guarded `TextProperty` change callback;
  - `ApplySpacerToDateTimeContent` finds `TimeInnerTextBlock` and `DateInnerTextBlock`, and requires a `StackPanel` parent.
- Line 896–1023: symbol hooks and module selection:
  - `DateTimeIconContent::OnApplyTemplate`, hooked on the implementation rather than the ABI thunk: calls the original, queries `pThis[1]` for `FrameworkElement`, then applies the spacer;
  - `BadgeIconContent::get_ViewModel` (optional): acts only when the runtime class is `SystemTray.DateTimeIconContent` and `IsLoaded()` is true;
  - `GetSystemTrayModuleHandle` picks `SystemTray.dll` first, then `Taskbar.View.dll` (only below version 2604; at 2604+ it returns `nullptr`), then `ExplorerExtensions.dll`;
  - a single `HookSymbols` call on an array commented `// SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll`; `TryHookSystemTrayModule` uses an `exchange` guard, then `Wh_ApplyHookOperations`.
- Line 1028–1043: `LoadLibraryExW` hook for when the tray module loads late — once the loaded module matches `GetSystemTrayModuleHandle()`, the tray hooks are installed.
- Line 1055–1075: `ClearSpacerStates` — for each entry, separately guarded: unregister the callback (and zero its token), restore the source block, remove the generated panel. Then it clears the vector.
- Line 1081–1181: Windhawk entry points:
  - `Wh_ModInit` hooks the tray module if it's already loaded; otherwise it hooks kernelbase's `LoadLibraryExW`, found with `GetProcAddress`;
  - `Wh_ModAfterInit` tries the tray hook again;
  - `Wh_ModUninit` sets `g_unloading` and returns early if the tray module was never hooked. Otherwise it runs `ClearSpacerStates` on the taskbar thread, retrying up to 5 times at 100 ms intervals while the window exists;
  - `ReloadSettings` and `Wh_ModSettingsChanged`: settings are reloaded and every line refreshed on the taskbar thread. Settings are loaded directly only when there's no taskbar window.

**Side effects of interest**

- None found. Everything the mod does stays inside its own process:
  - It hooks two SystemTray symbols, plus kernelbase `LoadLibraryExW` until the tray module loads.
  - It edits only this process's own taskbar XAML tree.
  - Its `EnumWindows` is filtered to the current PID, and its `WH_CALLWNDPROC` hook and registered message target only this process's taskbar thread.
  - It does no file, registry or network access, uses no IPC, and changes no system settings.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
