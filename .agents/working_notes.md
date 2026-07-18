# Working Notes — Windhawk Mod Lab

Current state only. Durable rules (standing directives, versioning, docs
policy) live in [README.md](README.md) — read that first.

## Current focus

Five mods to prepare (no submissions until each one works as described and its
docs/screenshots are current — see standing directives in README.md).

1. **Clock Spacer** — STALEMATE. PR #4443 (standalone) redirected by m417z to his dev repo. Integration PR #68 (m417z/my-windhawk-mods) stalled: m417z wants a TextAlignment=Justify + non-justifying-spaces approach; we couldn't get it to work and fell back to generated Grid/star-column layout which he won't accept for complexity. Left research at `taskbar-clock-customization-spacer/` and `_archive/clock-spacer/` for him to adapt if he chooses. **No further action planned.**
   - Important discovery: `%s%` at line edges is useful. `%s%content` right-aligns, `content%s%` left-aligns, `%s%content%s%` centers. This makes Clock Spacer a per-row alignment tool, not only a between-token gap tool.

2. **Taskbar Virtual Desktop Switcher** — v1.5 published as `taskbar-vd-switcher` (PR #3932, merged 2026-06-17). **v1.7 live-tested by user 2026-07-17 (highlight full-rebuild fix, accent default, generics, multi-monitor toggle) and pushed to PR #4516**, superseding v1.6 and clearing m417z's changelog ask. CI green, awaiting merge. Verified 2026-07-17: PR branch `update-taskbar-vd-switcher` matches the local lab file byte-for-byte. **Next: verify whether the full-rebuild change also fixes issue #4784 (hover/hit-test), refresh screenshots, sync folder README (settings table outdated).**

3. **Taskbar Folder Menus** — **v0.6 live-tested and pushed to PR #4485 2026-07-17** (all 4 m417z review items + crash-on-disable fix via `ClearButtonEventState()` + color generics + hover default `accent`); review reply posted, CI green after adding the "Mod authorship" PR-body section. Awaiting review. Verified 2026-07-17: PR branch `sb4ssman-taskbar-folder-menus` matches the local lab file byte-for-byte. Crash-fix background: XAML defers removed-subtree teardown to a later UI tick; Click delegates/tooltips/boxed Content pointed into the unloaded mod DLL (same hazard documented in m417z's taskbar-ai-quota); retests must run with the Windhawk log window CLOSED (open log masks the race).

4. **OmniButton Customizer** — active development. New in latest build (4e5e347):
   - `batteryPercentMode: independent` — battery glyph and percent placed as independent grid items at any position. Battery CP spans full grid; glyph at (bCol*slotW, bRow*slotH), percent at ((pCol-1)*slotW, pRow*slotH) absolute within CP.
   - Per-glyph colors: `wifiColor`, `volumeColor`, `batteryColor`, `percentColor`. Searches for `InnerTextBlock` by name, fallback to first TextBlock in subtree.
   - Animated colors: `wifiColorTo` etc. + `colorAnimateDuration` — WinRT Storyboard+ColorAnimation looping pulse.
   - Unknown slot logging for future Windows builds.
   - **2026-07-17: generic color tokens added** to `ParseHexColor` (Color-returning canonical parser: accent family + transparent; bare hex kept for back-compat; hex-digit validation added). Compile-checked OK. Folder README regenerated from embedded block (old one predated 4e5e347 features and claimed removed `wifiX`-style aliases); dev test checklist moved to `.agents/knowledge/omnibutton-test-checklist.md`. Root screenshots (`screenshot-inline/stacked/off.png`) are from the vertical era — need refresh for grid/color features.
   - PR #3859 (vertical-omnibutton) still open — can retire once customizer is submitted.
   - **User test round 1 (2026-07-17, dual-height taskbar)**: compiles and runs; survives repeated enable/disable cycles. Findings: (a) smart grid "never worked quite right" — root cause was `ResolveGeometry`: naive division with a min-2-rows clamp, no waste rebalancing, and the default `gridColumns: 2` meant auto never engaged; (b) **does NOT disable cleanly when battery percent is present** — "80%" left stranded top-right while the icons return to a row (screenshot evidence); without battery percent, disable restores cleanly, so the percent element is the culprit.
   - **Fixes applied 2026-07-17 (compile-checked, UNTESTED)**: `ResolveGeometry` rewritten — full-auto mode (both grid settings 0, now the default) prefers a single column whenever all items fit the taskbar height (24px unit), otherwise adds columns (4 items on single-height → 2x2); fixed-cols and fixed-rows overrides now derive the other axis from item count instead of an unrelated height division. Cleanup hardening for the stranded percent: `ClearTransformsRecursive` sweeps the whole battery subtree for stale RenderTransforms on disable (hypothesis: Windows re-templates the percent TextBlock so the cached ref no longer matches the live element). If the stranded 80% persists after this, next suspects: inner battery StackPanel orientation restore, and percent discovery landing on a different StackPanel in the dual-height template.
   - **User test round 2 (2026-07-17)**: battery percent no longer displays (off-screen or absent) after toggling the Windows battery-percent switch. Root cause found by code read: when the battery inner panel exists but the percent TextBlock hasn't materialized yet (exactly the toggle-while-applied case), the deferred LayoutUpdated path never re-scanned — layout stayed 3-item and the late percent rendered outside the 32px-wide battery CP.
   - **Fixes applied 2026-07-17 (compile-checked, UNTESTED)**: (1) `needsDeferred` and `OnLayoutUpdated` now also watch for a late percent TextBlock (inner panel present, `g_batteryPercentFE` null, inner SP child count reaching 2) and re-apply layout when it appears; `allReady` requires the percent before revoking the handler (handler stays registered on percent-less machines — same precedent as waiting for battery on desktops). (2) Coupled mode now sets battery CP `HorizontalContentAlignment(Left)` so the `(pCol-bCol-1)*slotW` percent math is exact instead of depending on native centering.
   - **ANIMATIONS REMOVED 2026-07-17 by user direction** ("I wanted something different — no animations"): `*ColorTo` settings, `colorAnimateDuration`, and all Storyboard machinery stripped. Feature archived at `.agents/knowledge/omnibutton-color-animation-archived.md` (recovery: `git show 2f6ec2c:...`); possible future addition after a design conversation. The feature had been added by an earlier agent session in 4e5e347.
   - **Template refactor 2026-07-17 (compile-checked, README_MATCH, UNTESTED)**: grid math replaced by a verbatim `_templates/smart-grid-layout.h` copy (`windhawk_mod_templates::smart_grid` namespace) — `ResolveGeometry` now derives GridMode from gridRows/gridColumns (0/0 = AutoSmart with availableRows from taskbar height ÷ 24px unit) and `GetCell` replaces `ItemGridPos`+`ShortGroupCenterNudge` (short-group placement now uses fractional offset units; behavior-equivalent for center/end). Added `shortGroupPosition` setting (first/last, default last). Settings block reordered to the canonical profile order (item list → grid → dimensions → colors → padding → per-item nudges). `fillOrder` option value is now `columnFirst`.
   - **User test round 3 (2026-07-17, fresh template build, battery percent on): FAILED.** Percent problems persist; independent mode makes the battery glyph disappear entirely. The late-percent LayoutUpdated hypothesis was wrong or incomplete. Grid math itself not implicated (user skipped grid testing, trusts the template).
   **Next: STOP static guessing. Get Windhawk logs from the user's machine first (`[Battery]`, `[Layout]` lines — item count, children count, coupled/indep offsets) and diagnose from evidence. Then fix percent + independent mode, and only then the rest of the checklist (grid retest, colors, screenshots, PR).**

5. **Privacy Indicator Anchor** — v0.9 in-tree (notes previously said v0.7 — stale). Full Option C grid: `itemOrder` (comma-separated token list replaces show* booleans and layoutMode), `gridColumns`, `fillOrder` (rowFirst/colFirst), `shortGroupPosition` (first/last), `shortGroupAlign` (center/start/end). `ComputeIconPlacement()` helper handles all arrangements. Camera experimental (0xE722, requires NoPhysicalCameraLED). Copilot token also present. Test page updated with camera + mic+cam combined section.
   - **2026-07-17: all four template-audit fixes applied**, compile-checked OK: (1) tooltip + automation-name clear at top of `RemoveSyntheticIcons` (folder-menus crash class); (2) `g_loadedRevokers.clear()` moved inside `RunFromWindowThread` in `Wh_ModUninit`; (3) `activeColorEnabled`+R/G/B replaced by single `activeColor` string; (4) `slashColor` now uses the canonical `ParseColorToken` (`#`-prefixed hex + generics — bare hex like `DC1E1E` no longer accepted; unpublished, no aliases). Both readmes: stale alias paragraph removed, copilot token documented, Colors section added.
   - **User test round 1 (2026-07-17, dual-height taskbar)**: compiles and runs; survives repeated enable/disable cycles; the 3-icon triangle looks right for a single-height taskbar.
   - **Changes 2026-07-17 (compile-checked, UNTESTED)**: `copilot` added to the default `itemOrder`; `gridColumns` default changed to `0` = auto — single column when the whole icon stack (N·iconSize + gaps) fits the taskbar height (double-height), otherwise two columns (the single-height triangle). Explicit 1/2/3+ still available.
   - **User test round 2 (2026-07-17)**: grid OK, but hardware camera (and hardware mic/location) activity never lights the icons — the mod only tracked native tray glyphs, which Windows never shows for hardware-LED cameras. Copilot slash appears at startup then clears ≈10s later: that is by design (state initializes disabled=true; first hardware poll detects the Web Experience Pack and clears it; dim-no-slash = installed/idle). Slash direction: verified geometry + `rising` default byte-identical in every commit that has contained it — no code change; if the user sees falling, check the saved `slashDirection` value.
   - **Added 2026-07-17 (compile-checked, UNTESTED)**: ConsentStore usage-record in-use detection — `CheckCapabilityInUse(webcam/microphone/location)` scans `CapabilityAccessManager\ConsentStore\<cap>` (packaged + NonPackaged, HKCU + HKLM) for `LastUsedTimeStart` set with `LastUsedTimeStop == 0` (the same signal Settings > Privacy uses for "currently in use"); polled every 3s on the existing thread, ORed with the glyph-driven active flags (`g_locUsage`/`g_micUsage`/`g_camUsage`). Covers hardware camera regardless of LED. Cellular: no Windows privacy indicator exists for it; a WWAN-presence token would be a new feature, deferred.
   - **Template refactor 2026-07-17 (compile-checked, UNTESTED)**: `ComputeIconPlacement` (hand-inlined template copy) replaced by the verbatim `_templates/smart-grid-layout.h` block — same namespace + `namespace grid` alias as omnibutton, so both mods' grid guts are now identical. Added `gridRows` setting (0 = auto); auto shape now comes from `ComputeLayout` AutoSmart with availableRows from taskbar height ÷ (iconSize + spacing) — same single-column-on-double-height / 2-cols-on-single-height results as the old hand rule. `fillOrder`/`shortGroupPosition`/`shortGroupAlign` now parse to template enums at load (`colFirst` accepted as legacy spelling of the new `columnFirst`). Settings block reordered to canonical profile order (position → items → grid → icon size/spacing → state styling → group geometry → per-item offsets → advanced).
   - **User test round 3 (2026-07-17, fresh build): FAILED.** (a) Camera still not detected — user has a hardware kill switch; ConsentStore usage detection and/or the disabled-slash did not reflect it. (b) Copilot slash still shows then clears on a machine with NO copilot (disabled incl. group policy) — `CheckCopilotInstalled` false-positives (stale package registration with surviving path?) and `CheckCopilotDisabled` never checks the `TurnOffWindowsCopilot` group policy. Known gaps, unfixed.
   **Next: STOP static guessing. Get Windhawk logs first (`[Cam]`, `[Copilot]`, `[Usage]`, `[Poll]` lines) to see exactly what the checks return on this machine; add the `TurnOffWindowsCopilot` policy check; then retest.**

---

## Readme/screenshot sync audit (2026-07-17, via `_templates/verify-readme-sync.ps1`)

- **taskbar-vd-switcher**: README_MATCH; 16 screenshots in assets/, but flagged for refresh post-v1.7 (accent default changes visuals).
- **taskbar-folder-menus**: README_MATCH; 4 screenshots at folder root, 3 referenced in both readmes.
- **omnibutton-customizer**: was MISMATCH → fixed 2026-07-17 (folder README regenerated from embedded). Screenshots outdated (vertical era).
- **privacy-indicator-anchor**: MISMATCH remains by design — embedded is user-facing, folder README is repo-facing (files list, status). Unify before submission. 1 image in assets/.
- **taskmanager-tail**: MISMATCH — published mod, folder README is a standalone repo README (install link, license) predating the sync policy. Needs a user decision on unification direction; no screenshots (non-visual mod, likely fine).
- **tray-utility-customizer**: MISMATCH — folder README (80 lines) much richer than embedded (30). Not yet read carefully; queue for a dedicated pass.
- **taskbar-clock-customization-spacer**: MISMATCH — embedded (100 lines) richer than folder README (50). STALEMATE mod, no action planned.
- **system-tray-grid-lines**: no .wh.cpp at folder root — script can't check it.

## Color settings rubric (decided 2026-07-17)

- 5 canonical slots in order: text, background, hover bg, pressed bg, border — then border thickness, corner radius, opacity, shine. **Optional identity axis** (e.g. VDS active/inactive): text and background split per identity in place; hover/pressed/border never split.
- 4 documented generics: `accent`, `accentLight` (AccentLight2), `accentDark` (AccentDark1), `transparent`. Numbered shades `accentLight1`–`3`/`accentDark1`–`3` accepted silently, undocumented. Defaults must be generics or empty — never hardcoded hex (folder-menus hover default changed `#4488FF` → `accent`).
- Canonical parser + convention in `_templates/button-surface.h` and `_templates/settings-profiles.md`. Applied to vd-switcher + folder-menus (incl. VDS YAML reorder to canonical slot order).
- **Full audit of remaining mods done 2026-07-17** — see the dated section in `_templates/six-mod-settings-audit.md`. Action queue:
  - omnibutton: DONE 2026-07-17 — generic token table in `ParseHexColor` (Color-returning variant); teardown already sound.
  - privacy-anchor: DONE 2026-07-17 — all four items (tooltip/automation-name clear in `RemoveSyntheticIcons`; revoker clear on UI thread; `activeColor` string; `slashColor` canonical parser). Untested in Windhawk.
  - tray-utility: conformant; optionally add `shortGroupPosition`.
  - Teardown contract now documented in `_templates/taskbar-xaml-lifecycle.template.cpp`; compile-check script at `_templates/compile-check.ps1`.

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

Notes: old OmniButton agent notes were later consolidated into `.agents/knowledge/`.
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
