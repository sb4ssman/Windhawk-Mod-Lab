# OmniButton regression investigation — 2026-09-09

## Follow-up: recovered after detailed logging and another reload

User supplied a full successful-load trace, beginning 06:49:09, and reports
that several earlier disable/enable cycles did nothing. After selecting detailed
logging and cycling again, the icons behaved. Earlier failure attempts are
not present in the supplied capture.

- Same Explorer PID 15272 and Windows build; no Explorer restart was needed.
- Both taskbar and SystemTray hooks resolve entirely from the existing caches;
  no symbol download or repair appears in this trace.
- Init starts 06:49:09.907; AfterInit completes .950; arrangement and monitoring
  are established .985. Discovery finds the original three presenters, battery
  StackPanel, Grid glyph and BatteryTextBlock. All four styling surfaces resolve.
- The exact saved expression produces four visible items, a 41 x 96 DIP
  footprint on a horizontal 96-DIP taskbar at 96 DPI. This is a vertical icon
  arrangement on a horizontal taskbar, not the vertical-taskbar stand-down.
- At 06:49:16.626, battery percentage changes 45% to 46%; the layout monitor
  detects it and successfully reapplies by .632.
- Follow-up read of Windhawk's current status confirms `explorer.exe|Loaded`.
- Registry logging flags changed from ordinary logging (`LoggingEnabled=1`)
  to detailed logging (`LoggingEnabled=0`, `DebugLoggingEnabled=1`). The mod's
  arrangement/settings timestamp did not change.

### What the logging selector actually changes

Inspected the installed UI extension source map and upstream engine source
pinned to **v1.7.3**, matching the installed engine:

- [mod.cpp, ApplyChangedSettings](https://github.com/ramensoftware/windhawk/blob/v1.7.3/src/windhawk/engine/mod.cpp#L2544):
  a logging-only config change updates the engine's logging flags for a loaded
  mod; it does not itself call the mod's settings callback or force a DLL reload.
- [mod.cpp, HookSymbols](https://github.com/ramensoftware/windhawk/blob/v1.7.3/src/windhawk/engine/mod.cpp#L1941):
  **either** ordinary or detailed logging bypasses cached symbol-failure
  throttling. Without either, failed symbol resolution can be cached for four
  hours at startup or one minute for later loads. Since ordinary logging was
  already enabled in our earlier snapshot, this does not establish an
  ordinary-to-detailed recovery mechanism here.
- Detailed logging adds verbose engine scopes around settings reads, hooks,
  initialization, etc. It can perturb timing, but this is not proof of a race.
- [customization_session.cpp](https://github.com/ramensoftware/windhawk/blob/v1.7.3/src/windhawk/engine/customization_session.cpp#L331):
  config notifications wait 200 ms before reconciliation. A sufficiently quick
  off/on may be observed only in its final enabled state.
- [mods_manager.cpp](https://github.com/ramensoftware/windhawk/blob/v1.7.3/src/windhawk/engine/mods_manager.cpp#L86)
  together with ApplyChangedSettings can retain an unloaded mod entry after a
  failed load without retrying on every unrelated config change. A disable
  actually observed by the engine removes the entry; re-enable creates a new
  load. This is a plausible recovery path, not a demonstrated history of the
  user's unsuccessful toggles. The original failure reason remains unknown.

**Assessment:** current Windows compatibility is demonstrated for this layout
and percentage updates. Persistent structural breakage is not supported by the
trace. Intermittent engine loading and logging-sensitive timing remain distinct
hypotheses. Do not claim detailed logging fixed the mod or add arbitrary sleeps.

**Next controlled check requested:** keep capture active; select ordinary mod
logging; disable and wait 3 seconds; enable and wait 10 seconds. Record whether
the layout returns, preserving a failed attempt if one occurs. No code or
installed configuration was changed by the agent.

## Confirmed evidence

- User reports all other mods working; OmniButton previously arranged the
  network, volume, battery and optional percentage vertically.
- Windows registry: 25H2, build 26200.9278. Explorer PID 15272, started
  2026-09-08 11:13 local time. Windhawk engine configured as 1.7.3.
- Installed source `C:/ProgramData/Windhawk/ModsSource/local@omnibutton-customizer.wh.cpp`
  is byte-identical to the lab source (SHA256
  `90AC4F8A4C6D49FD28CA3CBCA992A5305BCB8CF39F98631903210F67311432FF`).
- Installed library: `local@omnibutton-customizer_2.0_309456.dll`, built Aug 4.
  Registry has `Disabled=0`, `LoggingEnabled=1`, `Version=2.0`.
- Saved arrangement is intact:
  `network[-2,6], volume[0,2], battery[0,0], percent[2,-6]`.
  All four Content switches are on; ItemHeight=24, ItemWidth=0, PadX=6.
- Crucially, the current Windhawk mod-status file reports
  `explorer.exe|Unloaded` for OmniButton. All other enabled taskbar mods report
  `explorer.exe|Loaded`. Explorer's module list also lacks the OmniButton DLL.
  This is an observation, not proof that initialization failed: the reason
  for the unload still needs a fresh engine/mod log.
- SymbolCache contains resolved entries for all five taskbar hooks and
  `SystemTray::implementation::IconView::IconView` for the newer cached PDBs:
  taskbar `EB0D05FD4C8B8C721BFCBF0624FCD2881`, SystemTray
  `0689135353BA46BF8CAC790B2BB0514B1`. Both PDB files date to Aug 29.
  Cache presence alone does not establish a successful current hook/load.
- Explorer loads `SystemTray.dll` 2607.28000.0.0 and `Taskbar.View.dll`
  2607.28001.200.0 from MicrosoftWindows.Client.Core. The mod prioritizes
  SystemTray.dll, so its intentional rejection of newer Taskbar.View.dll
  is not itself evidence of the cause.
- Saved Aug 4 debug logs show successful arrangement and monitoring.
  Sept 5/8 UI log directories contain no new OmniButton debug trace.

## Windows and upstream context

[Microsoft KB5120998](https://support.microsoft.com/en-us/servicing/os/windows-11/2026/08/kb5120998-windows-11-24h2-25h2-update)
(Aug 27) corresponds to build 26200.9278. It changes taskbar position/size
options and system-tray loading reliability. This is a plausible regression
trigger, not an established causal diagnosis.

[PR #4855](https://github.com/ramensoftware/windhawk-mods/pull/4855) remains
open at `a6bde2de8156bfe20e55f1f2d7df1b853c7f9a2c`, with five successful checks.
No new maintainer diagnosis follows the July 23 review. That review mentioned
positional identification of network/volume as a future Windows compatibility
risk, but there is no live evidence yet that it is involved here.

## Next diagnostic step

User was asked to start OmniButton debug-log capture in Windhawk's Advanced
tab and toggle the mod off/on, then report completion. Read the newly created
`*-Windhawk Log.log` under `C:/ProgramData/Windhawk/UIData/user-data/logs/`.
Determine why it unloads before changing tree discovery or layout.

If it loads successfully but does not arrange, check `[Apply]` messages:
`GetTaskbarXamlRoot failed`, `ControlCenterButton not found`, or
`IsItemsHost StackPanel not found`. Current discovery assumes the direct
Grid > ContentPresenter > ItemsPresenter > StackPanel path. Inspect the live
tree before replacing it. Battery parsing also assumes a non-items-host
StackPanel with glyph then percentage; network/volume use slots 0/1.

No mod code, installed settings, or running processes changed in this
investigation. No fix claimed; no push or PR action taken.

## Reading live Windhawk status files

They are UTF-16LE and require shared access. Ordinary Get-Content fails while
Windhawk holds them. Open with FileAccess.Read and
`FileShare.ReadWrite | FileShare.Delete`, then use a Unicode StreamReader.
Filter filenames for the current Explorer PID; old files persist across boots.
