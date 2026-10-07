Source: https://github.com/ramensoftware/windhawk-mods/pull/5977#issuecomment-6030998612
Date: 2026-10-07T04:34:28Z

<!-- ai-review sha=76153d542e986d0094db4670c22455bcc886da60 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

This is a careful, read-only tool, and its unload path is clean: the worker thread is joined and every UI-thread dispatch is synchronous. Three things should be settled before merge:

**1. Persistent files written outside the mod's storage (lines 996–1021, 1131–1187; default at lines 102 and 467)**

The mod creates a folder and writes dump files to `%USERPROFILE%\Documents\Taskbar Tree Dumps` by default. It does this without any user action:
- about 2 s after every load. Because **Dump on load** is on by default, that includes every Explorer start or sign-in while the mod is enabled.
- after every settled taskbar change.
- after every settings save.

Each file is `taskbar-<date>-<time>-<edge>-<reason>[-<label>].txt|.json`, written with `CREATE_ALWAYS`. Nothing is ever removed: not on disable, not when the mod is removed, and the number of files has no cap. So everything written stays on disk whether or not uninit runs.

The dumps are the tool's output by design, so whether writing into a user folder is acceptable is the maintainer's call. A storage-scoped alternative:
- Make the default an empty **Output folder** that means "the mod's storage folder" (`Wh_GetModStoragePath`, which Windhawk removes with the mod). The path is already logged on each dump (line 1186).
- Keep a custom folder as an explicit opt-in.
- Say in the README that dump files stay on disk after the mod is disabled or removed.

```cpp
CopySetting(L"OutputFolder", raw, ARRAYSIZE(raw));
if (!raw[0]) {
    // Empty: the mod's own storage folder, removed together with the mod.
    Wh_GetModStoragePath(next.outputFolder, ARRAYSIZE(next.outputFolder));
} else if (!ExpandEnvironmentStringsW(raw, next.outputFolder,
                                      ARRAYSIZE(next.outputFolder))) {
    wcscpy_s(next.outputFolder, raw);
}
```

If you keep the Documents default, please explain why in the PR.

**2. It keeps polling every second, even when nothing will be dumped (lines 970–985, 1195–1216)**

Every second, for as long as the mod is enabled, the worker:
- calls `EnumWindows` twice,
- reads the registry,
- and, for **each** taskbar window (secondary ones too, which never return anything), installs a `WH_CALLWNDPROC` hook on Explorer's taskbar thread and makes a cross-thread `SendMessage`.

This continues after the load dump even when **Dump on change** is off, when the probe results are never used. Constant polling is a recurring objection for catalog mods. The README note "disable the mod when you are done" helps, but users don't reliably do that.

At minimum, stop polling when no automatic dump is wanted. Settings changes already wake the worker through `g_dumpNowEvent`, so the new timeout takes effect right away:

```cpp
AcquireSRWLockShared(&g_settingsLock);
bool polling = g_settings.dumpOnChange || (first && g_settings.dumpOnLoad);
ReleaseSRWLockShared(&g_settingsLock);
DWORD wait = WaitForMultipleObjects(2, waits, FALSE, polling ? 1000 : INFINITE);
```

An event-driven version would be better still. Rebuilds already come from the `TrayUI::StartTaskbar` hook. Moves and resizes could come from a `WM_WINDOWPOSCHANGED` subclass on `Shell_TrayWnd` (`WindhawkUtils::SetWindowSubclassFromAnyThread`), or from the XAML root's `SizeChanged` / the `DockingStates` group's `CurrentStateChanged`. Any of these would signal the worker, which then debounces.

**3. Catalog name and id**

`Windhawk-Mod-Lab Tool: Taskbar Tree Dump` / `mod-tool-taskbar-tree-dump` will show up in the catalog next to ordinary mods:
- A personal project brand that contains "Windhawk" reads like an official Windhawk component.
- "Tool" and the `mod-tool-` prefix collide with Windhawk's term "tool mod" (a mod that runs in its own `windhawk.exe` process), which this mod isn't.

A plain descriptive name works better, e.g. `Taskbar XAML Tree Dump` with id `taskbar-xaml-tree-dump`. Your description already says "For mod authors". The Mod Lab project can be credited in the README.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **Use the standard `RunFromWindowThread` snippet (lines 203–308).** Your version adds:
  - an atomic `g_dispatchMessage` whose comment mentions a "retry thread" that doesn't exist,
  - a `Dispatch::ran` guard for concurrent callers, although only the single worker thread ever dispatches,
  - a `SetExceptionLogger` indirection where calling `Wh_Log` directly would do.

  The version used across the repo ([taskbar-labels](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/taskbar-labels.wh.cpp#L316)) is shorter and easier to review. Keeping a `try`/`catch` around `proc` is fine. These additions look like AI artifacts; please confirm whether they're intentional.
- **Dead code and scaffolding.**
  - `ResolveTaskbarWnd` (lines 195–199) is never called.
  - The `==ModComponents==` namespaces with their aliases (`dispatch`, `taskbar_window`, `taskbar_xaml`) add structure that a single-file mod doesn't need.
  - `WorkerIteration` has a forward declaration (lines 1189–1191) and a leftover extra `{ }` block with double indentation (lines 1221–1245). It could be folded back into the worker loop.
- **`Wh_ModUninit` doesn't need the message pump (lines 1273–1283).** `Wh_ModUninit` always runs on the Windhawk engine thread. The worker only sends to the taskbar thread, never to that one. A plain `WaitForSingleObject(g_thread, INFINITE)` does the same job, and the comment there is misleading.
- **Includes.**
  - `<winrt/Windows.UI.Xaml.Controls.Primitives.h>` looks unused: every panel type used here is in `Windows.UI.Xaml.Controls`.
  - `std::abs(long)` in `EdgeByRect` belongs to `<cstdlib>`, which isn't included.
- **`ExpandEnvironmentStringsW` truncation (lines 468–470).** It returns the required size when the buffer is too small, and that case isn't detected, so an expanded path longer than `MAX_PATH` is silently cut. Check `result > ARRAYSIZE(next.outputFolder)`.
- **README and setting text for Label (lines 39, 83, 127).** It says the label is added to "the next file name". In fact it stays in every following file name until cleared.
- **README sample.** This mod has no on-screen effect, so a screenshot isn't needed. A short excerpt of the text output (a few lines, with the legend) would show at a glance what the tool produces.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **Auto-hide taskbars trigger repeated "change" dumps.** The signature includes each taskbar's absolute window rect (line 981). With auto-hide on, `Shell_TrayWnd` moves every time it slides in or out, so each reveal or hide that lasts at least 2 s writes another file. Combined with item 1, this fills the folder quickly. Comparing the window size plus the edge (`EdgeByRect`) instead of the absolute position would avoid this.
- **The load dump at Explorer start can be empty.** When the mod loads at process start, the worker starts before the taskbar exists. Two polls later it can write a "load" dump with zero taskbars, or with "XAML tree: not reached". A "change" dump follows once the taskbar appears. Waiting with the load dump until the primary root is reached (`probe.rootAbi != nullptr`) would give one useful file instead.
- **"Same element as the previous dump" relies on a raw pointer of a released object (lines 931, 951, 1163).** If a rebuilt root lands at the same address, the header wrongly reports "same element". The rebuild counter you already track (`g_rebuildCount`, recorded per taskbar at each dump) is a reliable signal for "rebuilt".
- **The tree walk runs entirely on the taskbar's UI thread.** It calls `TransformToVisual` for every element, up to 60000 elements, so the taskbar is briefly unresponsive during a dump. That's acceptable for a developer tool; this is just an FYI.
- **Overlap check:** no existing catalog mod does this. A couple of mods include bounded tree dumps to the log as debugging aids ([taskbar-multi-tray](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/taskbar-multi-tray.wh.cpp#L2376), [taskbar-brightness-and-opacity-tuner](https://github.com/ramensoftware/windhawk-mods/blob/2f67edf76611c6f463382cfbd6c987a4d6ee9d6b/mods/taskbar-brightness-and-opacity-tuner.wh.cpp#L910)), and UWPSpy is a live inspector outside Windhawk. A standalone, diffable file dump is a distinct niche.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `76153d5`. This mod is new, so everything below is new.

**Code regions**

- Line 1–11: metadata. `@include explorer.exe`, `@architecture x86-64`, links ole32/oleaut32/runtimeobject.
- Line 13–98: README.
- Line 100–139: settings: output folder, format (`$options` text/json), subtree, include text, label, dump on load, dump on change, max depth.
- Line 141–160: includes and `using namespace` for XAML.
- Line 162–201: `tree_dump_taskbar_window`. Finds this process's `Shell_TrayWnd` with a PID-filtered `EnumWindows`; `ResolveTaskbarWnd` is unused.
- Line 203–308: `tree_dump_dispatch`, a modified `RunFromWindowThread`. Installs a thread `WH_CALLWNDPROC` hook on the taskbar thread, `SendMessage`s a registered message (`Windhawk_RunFromWindowThread_<mod id>`), runs the callback inside the hook, then unhooks.
- Line 310–364: `taskbarDllHooks` on taskbar.dll, loaded with `LOAD_LIBRARY_SEARCH_SYSTEM32` and resolved in one `HookSymbols` call. Four address lookups (CTaskBand vftable, `GetTaskbarHost`, `FrameHeight`, `_Decref`) and one hook, `TrayUI::StartTaskbar`, which calls the original and then only bumps a rebuild counter.
- Line 366–430: `FrameworkElementOffset` (parses the `TaskbarHost::FrameHeight` prologue on x64/ARM64) and `GetTaskbarXamlRoot` (CTaskBand → TaskbarHost → FrameworkElement → XamlRoot). Same approach as the existing taskbar mods.
- Line 437–502: fixed-buffer `Settings` struct and `LoadSettings`: env-var expansion, label sanitized to file-name-safe characters, depth clamped to 1–200, published under an SRW lock.
- Line 504–512: state globals: unload flag, atomic rebuild counter, worker thread and event handles, element cap.
- Line 514–579: formatting helpers: printf wrappers, JSON string escaping, grid-length/thickness text.
- Line 581–781: element description. `Describe` reads layout properties, panel facts, transforms, TextBlock text (or only its length) and visual-state groups. `FindByName` finds the subtree root. `Collect` recurses up to the depth/element caps.
- Line 783–850: renders items as indented text lines or nested JSON.
- Line 852–911: enumerates taskbar windows (primary and secondary, PID-filtered), derives the edge from the rect, and reads the taskbar location and Windows build from the registry.
- Line 913–961: UI-thread callbacks. `ProbeOnUiThread` gets the root's identity and size; `DumpOnUiThread` does the subtree lookup and tree walk.
- Line 963–1021: change signature (runs the probe for each taskbar), recursive folder creation, UTF-8 file writer.
- Line 1023–1129: header and body rendering for text and JSON.
- Line 1131–1187: `Dump`. Gathers per-taskbar window, monitor and DPI info, runs the tree walk on the UI thread, builds the file name and writes the file.
- Line 1189–1246: worker thread. Polls every 1 s, dumps once the signature has held steady for two polls, forces a dump on settings change, and holds the settings lock shared for each iteration.
- Line 1248–1294: entry points. `Wh_ModInit` loads settings and resolves the symbols. `Wh_ModAfterInit` creates the events and the worker. `Wh_ModUninit` signals stop, joins the worker (with a sent-message pump) and closes the handles. `Wh_ModSettingsChanged` reloads settings and forces a dump.

**Side effects of interest**

- Line 996–1021, 1131–1187: external file write. Creates the output folder chain (default `%USERPROFILE%\Documents\Taskbar Tree Dumps`, any user-chosen path otherwise) and writes `taskbar-<date>-<time>-<edge>-<reason>[-<label>].txt|.json` with `CREATE_ALWAYS`. Runs on the worker thread about 2 s after load, after each settled taskbar change, and after each settings change. The files persist after disable and removal.
- Line 884–892: registry read: `HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced`, value `TaskbarLocation`, on every 1 s poll and every dump.
- Line 899–911: registry read: `HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion`, values `CurrentBuild` and `UBR`, on every dump.
- Line 266–306 (called from lines 979 and 1152): other: a thread-scoped `SetWindowsHookExW(WH_CALLWNDPROC)` on Explorer's own taskbar UI thread, installed and removed around each dispatch: once per taskbar window every second, plus once per taskbar for each dump. In-process only.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".
