# Working Notes — Windhawk Mod Lab

## Versioning convention

- `X.Y` = published / PR-ready version. Only bump `@version` header on a PR commit.
- `X.Y.Z` = internal save after confirmed-working local test. Tracked by git commit + tag (`mod/vX.Y.Z`).
- Between internal saves: just commit with descriptive message, no version bump.
- Archives: leave as-is, stop updating them. Git is the version history.
- Workflow: test → commit internal save (`X.Y.Z`) → when stable and ready to PR → bump to `X.Y+1` → PR.

---

## Current focus

1. **Clock Spacer** — STALEMATE. PR #4443 (standalone) redirected by m417z to his dev repo. Integration PR #68 (m417z/my-windhawk-mods) stalled: m417z wants a TextAlignment=Justify + non-justifying-spaces approach; we couldn't get it to work and fell back to generated Grid/star-column layout which he won't accept for complexity. Left research at `taskbar-clock-customization-spacer/` and `_archive/clock-spacer/` for him to adapt if he chooses. **No further action planned.**
   - Important discovery: `%s%` at line edges is useful. `%s%content` right-aligns, `content%s%` left-aligns, `%s%content%s%` centers. This makes Clock Spacer a per-row alignment tool, not only a between-token gap tool.

2. **Taskbar Virtual Desktop Switcher** — v1.5 published as `taskbar-vd-switcher` (PR #3932, merged 2026-06-17). v1.6 update PR #4516 open: https://github.com/ramensoftware/windhawk-mods/pull/4516. **Next: monitor review.**

3. **Taskbar Folder Menus** — v0.5, PR #4485 open: https://github.com/ramensoftware/windhawk-mods/pull/4485. Features: right-click Shell context menu, "Open in Explorer" in subfolder headers, lazy subfolder loading, `#4488FF` default hover. **Next: monitor review.**

4. **OmniButton Customizer** — active development. New in latest build (4e5e347):
   - `batteryPercentMode: independent` — battery glyph and percent placed as independent grid items at any position. Battery CP spans full grid; glyph at (bCol*slotW, bRow*slotH), percent at ((pCol-1)*slotW, pRow*slotH) absolute within CP.
   - Per-glyph colors: `wifiColor`, `volumeColor`, `batteryColor`, `percentColor` (hex #RRGGBB or #AARRGGBB). Searches for `InnerTextBlock` by name, fallback to first TextBlock in subtree.
   - Animated colors: `wifiColorTo` etc. + `colorAnimateDuration` — WinRT Storyboard+ColorAnimation looping pulse.
   - Unknown slot logging for future Windows builds.
   - PR #3859 (vertical-omnibutton) still open — can retire once customizer is submitted.
   **Next: test in Windhawk; verify independent mode math and color application; then PR as omnibutton-customizer.**

5. **Privacy Indicator Anchor** — v0.7 (in-tree, NOT committed). Full Option C grid: `itemOrder` (comma-separated token list replaces show* booleans and layoutMode), `gridColumns`, `gridFillOrder` (rowFirst/colFirst), `shortGroupPosition` (first/last), `shortGroupAlign` (center/start/end). `ComputeIconPlacement()` helper handles all arrangements. Camera experimental (0xE722, requires NoPhysicalCameraLED). Test page updated with camera + mic+cam combined section. Archive: tpia-test3.cpp.

---

## Mod infrastructure checklist (all system-tray mods must have)

- [ ] `GetModuleVersionInfo` helper  
- [ ] `GetSystemTrayModuleHandle()` (3-DLL chain: SystemTray.dll → Taskbar.View.dll w/ version check → ExplorerExtensions.dll)  
- [ ] `HandleLoadedModuleIfSystemTray` + `LoadLibraryExW` hook fallback  
- [ ] `WindhawkUtils::Wh_SetFunctionHookT`  
- [ ] `std::atomic<bool> g_systemTrayModuleHooked{false}`  
- [ ] `Wh_ApplyHookOperations()` after late-hook in both `Wh_ModInit` and `Wh_ModAfterInit`  
- [ ] `WaitForSingleObject(INFINITE)` on background threads in `Wh_ModUninit`  
- [ ] `@compilerOptions -lversion`  
- [ ] `GetTaskbarXamlRoot` uses `0x10` default + ARM64 disasm probe (NOT `0x48` + stub)  
- [ ] `iconView.Loaded` uses `g_autoRevokerList` auto-revoke pattern (NOT bare lambda)  
- [ ] No `%%`-unescaped `%` literals in `Wh_Log` format strings  

Status: omnibutton ✓, vd-switcher ✓, clock-spacer TBD, taskmanager-tail N/A (no system tray hook), folder-menu TBD, privacy-anchor TBD.

### OmniButton PR #3859 — status

Contacted m417z (2026-06-21) to propose retiring vertical-omnibutton in favor of omnibutton-customizer. Awaiting response. If he agrees, close #3859 and submit omnibutton-customizer as the single OmniButton mod. If not, the 5 inline review comments from 2026-05-09 still need a response commit (ARM64 probe, retry thread, %% escape, README wording, auto-revoke) — handled in separate chat.

---

## Repo consolidation — DONE (2026-04-27)

Both existing mod repos subtree-merged with full history:
- `omnibutton-customizer/archive/` ← Windhawk-Vertical-OmniButton history
- `taskmanager-tail/` ← Windhawk-Taskmanager-Tail

Notes: old OmniButton agent notes were later consolidated into root `_claude_notes/`.
Original repos left as-is on GitHub.

---

## Privacy Indicator Anchor — design notes

Motivation: Windows Web Experience Pack accesses location frequently; location icon pops in/out causing centered taskbar icon shifts. Existing `taskbar-tray-system-icon-tweaks` can only hide the icon entirely.

Approach:
- Target `MainStack` IconView elements (privacy indicators — mic/location)
- Register `TextProperty` callback on `InnerTextBlock` → update opacity on state change
- Register `VisibilityProperty` callback on element → override Collapsed → Visible (prevents system hiding)
- Active (text is privacy char): opacity 1.0; Idle (text empty): opacity = idleOpacity/100

XAML path: `SystemTrayIcon > ContainerGrid > ContentPresenter > ContentGrid > SystemTray.TextIconContent > ContainerGrid > Base > InnerTextBlock`

Active detection chars (from m417z's reference mod):
- `0xE37A` — Geolocation arrow
- `0xF47F` — Mic + Geo combined
- `0xE361`, `0xE720`, `0xEC71` — Microphone variants

Potential issues to test:
- Does the VisibilityProperty callback cause any flicker/conflict with m417z's system-icon-tweaks mod?
- Does the element exist in the XAML tree even when never used (privacy indicator never shown)?
- Does `MainStack > Content > IconStack > ItemsPresenter > StackPanel` path match current Windows builds?

## PR Review status

### PR #4484 — TO BE CLOSED

Submitted under wrong id (`virtual-desktop-switcher`). m417z confirmed ids are permanent. Close this PR and resubmit v1.6 improvements as an update to the existing `taskbar-vd-switcher` mod file.

### PR #4485 — Taskbar Folder Menus v0.5 (open, awaiting first review)

New mod. Both PRs passed CI after removing stray `vertical-omnibutton.wh.cpp` and renaming `hooks[]` → `taskbarDllHooks[]`.

---

### PR #3932 — Taskbar Virtual Desktop Switcher MERGED 2026-06-17 (v1.5, published)

**Comment 3175358025** — on `LoadLibraryExW_Hook`, hooks for `Taskbar.View.dll` / `IconView::IconView`:
> In newer builds, this function was moved to `SystemTray.dll`, see: https://github.com/ramensoftware/windhawk-mods/issues/3926
> Some mods were updated to support both older and newer builds. Please consider updating your mod as well.
→ ACTION: `GetSystemTrayModuleHandle` 3-DLL pattern needed (same as omnibutton v1.4). Already in v1.4 in-tree.

**Comment 3175381264** — on `GetTaskbarXamlRoot`, offset detection (x64-only, default 0x48):
> Use this updated code with 0x10 default offset and ARM64 support:
> https://github.com/m417z/my-windhawk-mods/blob/e1261d85c2f42006b0dc355fbbc3a8d71a078585/mods/taskbar-multirow.wh.cpp#L261-L291
→ ACTION: Replace offset block with the reference snippet (default `0x10`, full ARM64 disasm probe). Already in v1.4 in-tree.

**Comment 3175383064** — on `Wh_ModUninit`, `FreeLibrary(hSelf)` inside a `Dispatcher.RunAsync` lambda:
> That might be related to the crashes. Why call `FreeLibrary` here?
→ ACTION: Remove the `FreeLibrary` call. The DLL must not free itself from inside a pending async callback.

**Comment 3175387557** — on `Wh_ModAfterInit` retry `std::thread(...).detach()`:
> Is this really necessary? It will cause a crash if the mod is unloaded at this time.
→ ACTION: Replace detached thread with stoppable retry thread (same as omnibutton v1.4: `g_retryStopEvent` + `WaitForSingleObject` + join in uninit). Already done in v1.4 in-tree.

### PR #3859 — Vertical OmniButton (5 m417z inline comments from latest review 2026-05-09)

**Comment 3213784030** — on `GetTaskbarXamlRoot`, `#elif defined(_M_ARM64) // Use default offset.`:
> Please use the same code as can be found here:
> https://github.com/m417z/my-windhawk-mods/blob/e1261d85c2f42006b0dc355fbbc3a8d71a078585/mods/taskbar-multirow.wh.cpp#L261-L291
> In this mod and your other mod.
→ ACTION: The ARM64 arm must use the full disasm probe (4-instruction pattern check), not just "use default". Reference code (lines 261–291):
```cpp
size_t taskbarElementIUnknownOffset = 0x10;
#if defined(_M_X64)
    { /* ... sub rsp / add rcx pattern ... */
        taskbarElementIUnknownOffset = b[7];
    }
#elif defined(_M_ARM64)
    { /* pacibsp / stp fp,lr / mov fp,sp / ldr x8,[x0,#offset]! pattern */
        taskbarElementIUnknownOffset = (p[3] >> 12) & 0xFF;
    }
#else
#error "Unsupported architecture"
#endif
```

**Comment 3213836339** — on `g_retryThread` / `CreateThread` in `Wh_ModAfterInit`:
> The mod already hooks `IconView::IconView`, so why is this thread needed?
→ ACTION: Explain or remove the retry thread. Since `IconView::IconView` fires when OmniButton elements appear, the retry is redundant. Can remove the thread if the hook reliably fires on init.

**Comment 3213837729** — on `Wh_Log(L"... (% may not be in tree yet)")`:
> Use `%%` to escape the `%` sign.
→ ACTION: Change `%` in Wh_Log format strings to `%%` where literal `%` is intended.

**Comment 3213839585** — on README, `→ Settings → Advanced`:
> ```suggestion
> into Windows 11 Taskbar Styler → Settings → Textual mode.
> ```
→ ACTION: Accept the suggestion — change "Advanced" to "Textual mode" in the README block.

**Comment 3213844970** — on `iconView.Loaded([](...)` without unsubscribing:
> Unsubscribe after handling, otherwise the callback might be called again when the mod is unloaded, causing a crash. There's an example below, but if you prefer something simpler, allocate memory for the revoke token, assign it from `iconView.Loaded`, pass the pointer to the lambda, and unsubscribe when it's called.
> https://github.com/ramensoftware/windhawk-mods/blob/a5fb564ecbe5c4cca655a63a0562113123fd5b21/mods/taskbar-tray-system-icon-tweaks.wh.cpp#L1321-L1331
→ ACTION: Use `winrt::auto_revoke_t{}` pattern from that reference — store token in `g_autoRevokerList`, erase from within the callback. Or simpler: heap-allocate the revoke token, pass pointer to lambda, unsubscribe on first call.
Reference pattern:
```cpp
g_autoRevokerList.emplace_back();
auto autoRevokerIt = g_autoRevokerList.end();
--autoRevokerIt;
*autoRevokerIt = iconView.Loaded(
    winrt::auto_revoke_t{},
    [autoRevokerIt](IInspectable const& sender, RoutedEventArgs const& e) {
        g_autoRevokerList.erase(autoRevokerIt);
        // ... do work ...
    });
```

### Earlier PR #3859 comments (already addressed in v1.4)

- **3129967175**: XAML Diagnostics exclusive consumer conflict with Taskbar Styler → resolved by switching to `GetTaskbarXamlRoot` pattern
- **3129973528**: Registry write (`TaskbarBatteryPercent`) is persistent → removed in later revision
- **3130831004** (sb4ssman reply): acknowledged, confirmed both fixed

---

## Densification goal (2026-05-08)

The tray should be DENSE. Every column earns its pixels. Target layout (right-to-left from show-desktop):
`[show desktop] | [clock stats] | [OmniButton] | [privacy mirror] | [notification icons + chevron shared] | [VD buttons]`

Key ideas captured in `_research/densification-analysis.md`:
- **Chevron + privacy icon column sharing**: on double-height taskbar, stack mirrored privacy icon and native overflow chevron in one column. Privacy Indicator Anchor mod is the right home for this. Need to find chevron XAML element name first (inspect live tree).
- **Tray Stats Panel mod (future)**: free-standing stats panel injected into the tray, not tied to the clock area. Independent of Clock Customization. More flexible placement.
- **Clock Spacer width problem**: the mod needs Max Width to be set consistently. The `Max clock width` setting should handle this but needs verification. Consider auto-calibration from observed peak ActualWidth.
- **Clock Spacer → upstream submission**: ~200 lines of active code, could be proposed as direct addition to m417z's Clock Customization mod. Standalone PR first, then pitch.

## TODO (future)

- **Privacy Indicator Anchor filler/status idea** — consider a future mode that fills the two-icon vertical stack with useful status/controls when space allows, e.g. microphone decibel level and a small globe/location affordance that can help locate the device.
- **Taskbar Folder Menu chevron experiment** — explore replacing or sharing the show-hidden-icons chevron area with a small vertical stack of two or three folder buttons. Need inspect live XAML names for the chevron/overflow host and decide whether to hide the native chevron, wrap it with custom folder buttons, or inject adjacent to its parent.
- **windhawk-mods PR update script** — a script that: (1) pulls latest upstream, (2) copies updated .wh.cpp into mods/, (3) creates/updates PR. Fork at `t:/Github/sb4ssman/windhawk-mods/`.

## Current experiments

- Privacy Indicator Anchor tray-grid direction is documented in `_research/privacy-indicator-anchor-design.md`. Preferred implementation is a persistent mirrored icon near `NotifyIconStack`, not moving Windows' real privacy `IconView`.
- Taskbar Folder Menu prototype in `taskbar-folder-menus/`. Injects compact folder buttons into `SystemTrayFrameGrid`, opens native popup menus with `TrackPopupMenu`.
- VD Switcher: experimental Start-area positions are now `nextToStart`, `overStart`, and `rightOfStart`; legacy `aboveStart`/`belowStart` are hidden settings aliases of overlay mode.

## Completed

- taskmanager-tail v1.0 published (PR #3045), updated to v1.1 with Windows 10 support (PR #3143)
- taskbar-vd-switcher v1.0 PR submitted (#3932)
- vertical-omnibutton v1.2 PR submitted; v1.4 PR updated (#3859)
- repo consolidation done (2026-04-27)
