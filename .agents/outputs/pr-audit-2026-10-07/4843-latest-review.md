<!-- ai-review sha=8f5f1b08fdb1cba74b01ff9f786b966899cd3250 -->

### Submission review

_Note: This review was done by Claude. Due to the amount of submissions, doing a fully manual review for each pull request is no longer feasible. Thank you for understanding._

Remember: The AI reviewer can be wrong - it may misread code, flag correct code as broken, or suggest changes that make things worse. Treat its findings as suggestions to verify, not instructions to follow blindly. You're responsible for the code you submit, so if a finding doesn't hold up, say so instead of changing working code to satisfy it.

Please address the following issues. The items in the collapsed sections are optional, so it's your call whether to address them.

---

No blocking issues — looks good to merge.

All four items from the previous round check out at 8f5f1b0:
- The text callback now records `type`/`typeKnown` before either `SetPrivacyActive` call (5664–5673).
- The worker reads `g_syntheticBarLive` (6094), an atomic that is set and cleared next to `g_syntheticGrid` (5368, 5461).
- The helpers listed last time are gone.
- The visibility callback now refreshes the lease snapshot only for writes the mod didn't make. That is guarded by `g_ownVisibilityWrite`, which `ApplyNativeSuppression` sets around both its collapse and its restore (5559–5585, 5690–5696).

The new edge-following code also tears down correctly:
- Both `Wh_ModUninit` dispatches (6324, 6342) stop the `SizeChanged`/`CurrentStateChanged` subscriptions and the `g_edgeTimer` Tick on the UI thread, before anything else.
- Every entry point (`OnTaskbarEdgeChanged` 5862, the tick 5870) bails out on `g_unloading`.
- `g_edgeTimer` uses the bare `[[clang::no_destroy]]` form, which is correct for a nullable WinRT type. `g_edgeWatch` holds only weak refs and tokens, so it needs no attribute.

For the maintainer: this round renamed the mod ID from `tray-privacy-indicator-anchor` to `privacy-indicator-anchor`. That's harmless because the mod isn't merged yet.

<details><summary>Optional improvements</summary>
<p>

Minor polish — none of this affects users, so it's your call.

- **A failed settings dispatch leaves the mod half-updated.** `LoadSettings(next)` publishes `g_cameraHardwareDetectionEnabled`, `g_cameraItemEnabled` and `g_copilotItemEnabled` straight away (2689–2692). When the dispatch at 6390 doesn't run against a live window, `g_settings` keeps the old values (6403–6407). The worker then follows the new Camera/Copilot toggles while the bar keeps the old layout, and the only sign of it is a log line. `Wh_ModUninit` already handles the same failure by retrying once against `ResolveTaskbarWnd(nullptr)` (6339–6344). Doing that here would cover the realistic cause, a `Shell_TrayWnd` that was recreated between the resolve and the send. Alternatively, load the three atomics only when the copy is actually published.

- **A few leftovers:**
  - `namespace dispatch = privacy_anchor_dispatch;` (1464) is still there alongside `ui_dispatch` (2475), even though the PR comment says the duplicate alias was removed.
  - `LoadColorSetting` (2594) still has the `setting.get() ? … : L""` null check that was removed everywhere else.
  - `Wh_ModInit` logs `v2.0` (6029). `Wh_Log(L"[Init] Privacy Anchor v" WH_MOD_VERSION)` can't go stale.

</p>
</details>

<details><summary>Functionality notes</summary>
<p>

Non-critical observations and ideas about the feature behavior itself.

- **`leftOfStart`/`rightOfStart` on a side taskbar may be clipped to RootGrid's first row.** `start_placement::Acquire` spans the group across all of `RootGrid`'s columns (2421–2425) but leaves `Grid.Row`/`RowSpan` at their defaults. `Position` then moves the group along the lane with `Margin.Top` on a vertical taskbar (2353–2354). If `RootGrid` lays out by rows on a left or right edge, the group stays inside row 0, and a lead larger than that row's height gets clipped or pushed back. The README promises "above and below Start", and the side-taskbar screenshot shows only a tray position, so this is worth a quick check. If it does clip, also set `Grid::SetRowSpan(group, std::max(1, (int)rootGrid.RowDefinitions().Size()))` next to the column span.

- **`Lease::Refresh` can capture the mod's own value if Windows animates `Visibility`.** The visibility callback fires on any change to the effective value. Animations and visual-state setters outrank local values, so when one of them shows the icon, `ReadLocalValue` still returns the mod's own `Collapsed`. `Refresh` then stores that `Collapsed` in the snapshot (1279), and unload would "restore" a hidden indicator. This only matters if Windows drives `IconView.Visibility` through a storyboard or visual state rather than direct writes. A cheap guard is to refresh only when the local value is unset or matches the new effective value (`unbox_value<Visibility>(local) == iconView.Visibility()`). A mismatch means a higher-precedence source made the change, and the snapshot should stay as it is.

</p>
</details>

<details><summary>Code overview</summary>
<p>

For the maintainer: a map of the mod and what it touches outside its own process. Descriptive only — anything that needs a change is listed above. Line numbers refer to the mod source at commit `8f5f1b0`. This mod is new, so everything below is new.

**Code regions**

- Line 1–11: metadata — `explorer.exe`, `x86-64`, links ole32/oleaut32/runtimeobject/version/setupapi/cfgmgr32/shell32.
- Line 13–259: README — gallery, the Arrangement grammar (with order of operations), placement, taskbar position (native side edges supported, a taskbar rotated by another mod stands down), states/colors, notes on the camera monitor and native suppression.
- Line 261–419: settings block (Placement / Content / Layout / Size / Adjust / Surface / Behavior).
- Line 421–472: includes and `using` directives.
- Line 474–609: components — clamped setting readers and the `$options` choice table via `WindhawkUtils::StringSetting`, and the color-token parser (hex / accent / transparent).
- Line 611–1217: Arrangement expression — parser with a nesting limit, memoized measure/arrange, auto shape, `across` variant for side taskbars, append-missing policy.
- Line 1219–1316: property lease — snapshots a dependency property's local value before the first write and restores it (or calls `ClearValue`) per object or for all. `Refresh` (1268–1283) re-reads a snapshot after an external write.
- Line 1318–1459: taskbar window discovery (`EnumWindows` filtered to this process) and UI-thread dispatch through a transient `WH_CALLWNDPROC` hook plus a `RegisterWindowMessage` name that embeds `WH_MOD_ID`. The message is checked before `lParam` is trusted.
- Line 1461–1581: `taskbarDllHooks` → `taskbar.dll` (`CTaskBand` vtable, `GetTaskbarHost`, `FrameHeight`, `_Decref`, `TrayUI::StartTaskbar` hook → rebuild callback). Also the runtime-derived `FrameworkElement` offset and the XamlRoot walk.
- Line 1583–1834: taskbar metrics — `ReadDockedEdge` reads RootGrid's `DockingStates`; `GetMetrics` gives orientation, thickness in DIPs and a "rotated" flag (side-shaped while Windows reports a horizontal dock); `EdgeWatch` subscribes to TaskbarFrame `SizeChanged` (thickness/orientation only) and the `DockingStates` `CurrentStateChanged`.
- Line 1836–2135: tray slot lease — a Grid column or StackPanel index held by a named zero-size marker, with stale-marker recovery.
- Line 2137–2465: Start-adjacent placement — pushes `TaskbarFrameRepeater`, counter-shifts Start, follows `LayoutUpdated`. Works along x or y depending on the edge.
- Line 2482–2693: settings enums, `ModSettings` (fixed buffers, trivially destructible), and `LoadSettings`, which parses every `$options` value into an enum and publishes three atomics for the worker.
- Line 2695–2870: globals — state atomics, `[[clang::no_destroy]]` XAML holders (bare WinRT types; `std::optional` for the lease/revoker/event containers), `g_syntheticBarLive`, and `g_privacyStates` (weak refs only).
- Line 2872–3048: version-info helper; `GetSystemTrayModuleHandle` (SystemTray.dll → Taskbar.View.dll before build 2604 → ExplorerExtensions.dll); exception logger; dispatch wrappers; XAML tree helpers; privacy-glyph detection and the `MainStack` host filter.
- Line 3050–3214: layout glue — token resolution, `AvailablePrivacyRows` (counts lines across the taskbar's thickness, so width on a side taskbar), and `ComputePrivacyPlacements` (auto or written arrangement, parse-error fallback, append).
- Line 3216–3468: synthetic icon state — glow toggle, opacity and color per state, tooltips and automation names, settings URIs, `OpenSettingsForItem` (`ShellExecuteW` on tap), `SetPrivacyActive`.
- Line 3470–4345: background monitors:
  - `CheckMicBlockReason`.
  - `MicPrivacyMonitor` — `IMMNotificationClient` + `IAudioEndpointVolumeCallback`; unregisters, then detaches under a lock.
  - Camera init cancellation globals.
  - `CameraPrivacyMonitor` — opt-in `MediaCapture` in SharedReadOnly mode + `CameraOcclusionInfo`, with backoff.
  - `RegistryChangeMonitor` — thread-agnostic `RegNotifyChangeKeyValue` with backoff.
  - `DeviceStateMonitor` — `DeviceAccessInformation` + `DeviceWatcher`.
- Line 4347–4738: state probes — camera block reason (WinRT access, SetupAPI presence/problem, ConsentStore), Copilot installed/active/blocked, location block reason, and the ConsentStore usage-record scan.
- Line 4740–4804: `UpdatePrivacyStates` — refreshes the flagged domains on the worker and dispatches `UpdateSyntheticState` to the UI thread when something changed.
- Line 4806–4935: XAML builders — glyph `TextBlock`, glow halos/rings with storyboards — plus `ApplyOffset`.
- Line 4937–4987: edge globals (`g_edgeWatch`, `g_edgeTimer`) and `TaskbarRequiresStandDown` — reads the docked edge, starts the edge watch, sets `g_appliedSide`, stands down only on a rotated taskbar.
- Line 4989–5474: `InjectSyntheticIcons` (stand-down check, build the bar with per-item slots/tooltips/`Tapped`, lease a tray slot or the Start lane, publish with `UnwindPublishOnThrow`) and `RemoveSyntheticIcons` / `RemoveModUi`.
- Line 5476–5773: native indicator tracking:
  - `UntrackPrivacyElement`.
  - `SyntheticSlotReplaces` + `ApplyNativeSuppression` — the single collapse/restore decision, with the `g_ownVisibilityWrite` guard.
  - `ApplyPrivacyIndicatorBehavior` — text and visibility callbacks, `typeKnown`, snapshot refresh.
  - `ScanMainStack` and `ClearPrivacyStates`.
- Line 5775–5916: `ApplyStyle` / `ApplyOnTaskbarThread` / `ApplyStyleOnWindowThread`; `OnTaskbarEdgeChanged` (150 ms one-shot `DispatcherTimer` → `RemoveModUi` + re-apply); `StopRetryThread` (signal, cancel camera init, `MsgWaitForMultipleObjects` join).
- Line 5918–6022: hooks — `OnTaskbarRebuilt`, the `IconView::IconView` hook that registers a `Loaded` revoker, the `LoadLibraryExW` hook, and `systemTrayModuleHooks` → `// SystemTray.dll, Taskbar.View.dll, ExplorerExtensions.dll` (5998).
- Line 6024–6409: lifecycle entry points:
  - `Wh_ModInit` — settings, taskbar.dll hooks, then the tray-module hooks or the kernelbase `LoadLibraryExW` hook.
  - `Wh_ModAfterInit` — first apply plus the worker thread: 5 retries, then an event loop with a 60 s Copilot poll, 5 min reconciliation and a 2 s Copilot debounce.
  - `StopEdgeFollowing` + `Wh_ModUninit` — joins the worker, then on the UI thread stops edge following, runs `RemoveModUi` and calls `reset()` on the optionals, with a checked retry.
  - `Wh_ModSettingsChanged` — loads into a copy and publishes it on the taskbar thread.

**Side effects of interest**

- Line 3491–3514, 4429–4468, 4643–4682: registry read: `HKCU`/`HKLM\...\CapabilityAccessManager\ConsentStore\{microphone,webcam,location}` → `Value` ("Deny"), on each state refresh.
- Line 4693–4738: registry read: every subkey (and `NonPackaged\*`) under those ConsentStore keys in both hives → `LastUsedTimeStart` / `LastUsedTimeStop`, on each usage refresh.
- Line 4609–4642: registry read: `HKLM\SOFTWARE\Policies\Microsoft\Windows\LocationAndSensors\DisableLocation` and `HKLM\SYSTEM\CurrentControlSet\Services\lfsvc\Service\Configuration\Status`.
- Line 4569–4606: registry read: `HKCU`/`HKLM\Software\Policies\Microsoft\Windows\WindowsCopilot\TurnOffWindowsCopilot`, `HKCU\...\Explorer\Advanced\ShowCopilotButton`.
- Line 4481–4545: registry read: enumerates the HKCU and HKLM `AppModel\Repository\Packages` keys for `Microsoft.Copilot_*` / `Microsoft.Windows.Ai.Copilot_*` → `PackageRootFolder` / `Path`. Then **external file read**: a `GetFileAttributesW` existence check on that package folder (4533). Debounced to one scan per 2 s of AppModel churn.
- Line 4171–4223, 6117–6177: registry change notifications (`RegNotifyChangeKeyValue`, subtree, thread-agnostic) on 12 keys — the six ConsentStore keys, `HKCU`/`HKLM\Software\Policies\Microsoft`, `HKLM\...\Services\lfsvc`, `HKCU\...\Explorer\Advanced`, and both AppModel repositories. Re-armed after every signal. No registry writes anywhere.
- Line 3516–3548, 3551–3722: IPC: MMDevice COM (default capture endpoint state, `IAudioEndpointVolume` mute) — the out-of-process audio service. Registers `IMMNotificationClient` and `IAudioEndpointVolumeCallback`, both unregistered in `Cleanup` before the worker exits.
- Line 3480–3481, 4228–4345, 4358–4371: IPC: WinRT `DeviceAccessInformation` (mic/camera consent status + `AccessChanged`) and a `DeviceWatcher` over `VideoCapture` — out-of-process device/consent brokers. All tokens are revoked in `Cleanup`.
- Line 3930–3991: IPC: `MediaCapture::InitializeAsync` on the default camera in `SharedReadOnly` mode + `CameraOcclusionInfo::StateChanged` (Frame Server). **Opt-in** (`Behavior.CameraHardwareDetection`, default off), cancelled on unload, `Close()`d in `Cleanup`.
- Line 4376–4428: other: SetupAPI camera class enumeration (`SetupDiGetClassDevs`, `SPDRP_FRIENDLYNAME`, `CM_Get_DevNode_Status`) on each camera state refresh — read-only.
- Line 4548–4565: other: `CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS)` process-list scan for four Copilot executables, once a minute while the Copilot icon is on.
- Line 3447–3455: other: `ShellExecuteW(L"open", L"ms-settings:...")` — launches Settings, only when the user taps a synthetic icon.
- Line 1325–1342: other: `EnumWindows`, filtered to `GetCurrentProcessId()` + `Shell_TrayWnd` (read-only lookup).
- Line 1428–1455: other: `SetWindowsHookExW(WH_CALLWNDPROC)` scoped to the taskbar thread of this process, installed and removed around a single `SendMessage`.
- Line 1498–1499: other: `LoadLibraryExW(L"taskbar.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32)` (already loaded in Explorer; restricted to System32).
- Network access: none found. Persistent writes: none — every change is to the taskbar's live XAML tree and is reverted in `Wh_ModUninit`.

</p>
</details>

---

**Next steps:**

* `/ai-review` - after pushing fixes, to get a review of the updated code. You can repeat this as many times as you need, but each review is thorough and usually there's no need for more than 2-3 iterations.
* `/ready-for-reviewer` - once you're satisfied with the state of the pull request, to hand it over to a human reviewer. If some findings above are left unaddressed, add a short note explaining why.

See the [review process](https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process) for details.

Important: Each reply that was generated by AI must be disclosed by including the following text in the reply: "Generated by AI. Model: (ai-model-name)".