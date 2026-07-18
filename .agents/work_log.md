# Work Log — Windhawk Mod Lab

Lab-level milestones only, chronological, newest at the bottom. Per-mod
historical notes live in `.agents/knowledge/` with mod-prefixed filenames.

## 2026-04-27 — Lab established

- Created Windhawk-Mod-Lab as umbrella repo
- Subtree-merged Windhawk-Vertical-OmniButton history, now retained under `omnibutton-customizer/archive/`
- Subtree-merged Windhawk-Taskmanager-Tail → taskmanager-tail/
- Added profiles/ folder for per-machine Windhawk config snapshots
- VD switcher design doc in place; implementation not yet started

## 2026-05-05 — Clock Spacer live tuning

- Clock Spacer v0.7 is visually close with multiple `%s%` instances across top and bottom clock lines.
- Current tested lines:
  - Top: `🤖%cpu%🍵%ram%%s%%time%`
  - Bottom: `%weekday%%s%📅%s%%date%%n%🛫%upload_speed%%s%🛬%download_speed%%n%🧮%gpu%🧮%gpu%%s%💽%disk_read%%n%%weather%`
- Known limitation: `%s%` spacer handling does not work inside the weather string.
- Discovery: `%s%` at line edges is an alignment tool — `%s%content` right-aligns, `content%s%` left-aligns, `%s%content%s%` centers.

## 2026-05-08 — Maintainer review pass (session 2)

### Vertical OmniButton → v1.4 (PR #3859 updated)
- Applied GetSystemTrayModuleHandle 3-DLL pattern
- Added GetModuleVersionInfo helper
- ARM64 arm in GetTaskbarXamlRoot
- WindhawkUtils::Wh_SetFunctionHookT throughout
- std::atomic<bool> g_systemTrayModuleHooked
- Wh_ApplyHookOperations after late-hooks
- Inline mode auto-sizing fixed
- HandleLoadedModuleIfSystemTray + LoadLibraryExW fallback

### Clock Spacer → v0.8 (in-tree, not yet committed — needs test confirmation)
- Fixed restart persistence: system tray DLL hook not surviving explorer restart
- Added GetModuleVersionInfo + GetSystemTrayModuleHandle (with Taskbar.View.dll version check)
- Added LoadLibraryExW hook + HandleLoadedModuleIfSystemTray
- g_systemTrayModuleHooked → std::atomic<bool>
- Wh_ApplyHookOperations() in Wh_ModAfterInit after late-hook
- Added -lversion to compilerOptions

### Virtual Desktop Switcher → v1.4 (in-tree, SET ASIDE — layout bug under investigation)
- All infrastructure: GetSystemTrayModuleHandle, ARM64, atomic hook flag, Wh_ApplyHookOperations
- Crash-on-disable: StopNotificationThread now waits INFINITE
- Half-button-clickable: Canvas::SetZIndex(100) on injection
- Stale parent on desktop-add: RebuildOrUpdate uses FindLiveSystemTrayFrameGrid + ApplyAllSettings fallback
- Must-move-mouse: SwitchToDesktop dispatched to background COM thread
- Sliver height: subtracted from GetAvailableRows when sliver mode active
- Layout scoring: only penalize wide layouts (cols>rows), not tall (rows>cols)
- Layout diagnostics: Wh_Log in GetAvailableRows and ComputeLayout
- Outstanding: 2-column layout on 3-desktop restart — needs log inspection to determine if height detection or scoring issue

## 2026-05-09 — Maintainer review applied (PR #3859 + #3932)

### Vertical OmniButton
- ARM64 disasm probe in `GetTaskbarXamlRoot` (default `0x48` → `0x10`, full 4-instruction pattern check)
- `iconView.Loaded` → auto-revoke pattern via `std::list<FrameworkElement::Loaded_revoker> g_autoRevokerList` (prevents crash-on-unload from stale callbacks)
- `%` → `%%` escape fix in one `Wh_Log` call
- README: `→ Settings → Advanced` → `→ Settings → Textual mode.`

### Virtual Desktop Switcher
- ARM64 disasm probe in `GetTaskbarXamlRoot` (same fix)
- `rightOfStart` position added (grid floats right of Start button)
- `aboveStart` width expansion: pushes `TaskbarFrameRepeater.Margin.Left` by `(gridW−startW)/2` to center Start under VD grid; uses `g_startButtonOriginalX` stable anchor to prevent `LayoutUpdated` oscillation
- `gridVerticalOffset` setting now applies in all overlay modes (previously only applied in tray-injection path)
- Sliver distortion fix: explicit `VerticalAlignment::Top` + computed `marginTop` replaces `Center`+compensation approach
- Fill order default: `columnFirst` → `rowFirst`
- `masterButtonSpacing`: sentinel model removed, pure additive margin offset
- Row-first short-group centering: pixel-precise via column-span + `Margin.Left` (mirrors column-first approach)

## 2026-05-08 — Densification goal set

Target tray layout (right-to-left from show-desktop):
`[show desktop] | [clock stats] | [OmniButton] | [privacy mirror] | [notification icons + chevron shared] | [VD buttons]`
Analysis captured in `_research/densification-analysis.md`.

## 2026-06-15 — Clock Spacer PR readiness pass

- User confirmed Clock Spacer has been working and toggled off/on several times without issues.
- Hardened the initial scan thread: it now has a stop event and is waited during unload with a sent-message pump.
- Documented the weather-string limitation and added a maintainer note that the `%s%` token could be absorbed into Taskbar Clock Customization if desired.
- Bumped Clock Spacer to v1.0 for submission.
- Copied `clock-spacer.wh.cpp` to `windhawk-mods/mods/taskbar-clock-spacer.wh.cpp`, committed as `89f1bc6f` on branch `add-taskbar-clock-spacer`, pushed to fork, and opened PR #4443: https://github.com/ramensoftware/windhawk-mods/pull/4443
- Root README now marks Clock Spacer v1.0 as PR submitted.

## 2026-06-17 — VD Switcher Start-placement rethink

- Collapsed `aboveStart` / `belowStart` into legacy aliases of `overStart`; settings UI now exposes only left of Start, over Start, and right of Start.
- Made overlay mode pure overlay; vertical placement is controlled by `gridVerticalOffset` instead of separate above/below placement modes.
- Reworked `rightOfStart`: pushes `TaskbarFrameRepeater.Margin.Left` to reserve room and applies a visual X counter-offset to the Start button so Start stays anchored while taskbar items move right.
- Submission-prep pass: incorporated renamed screenshots into README and Windhawk readme, documented tested left/over/right Start examples, removed stale Start-placement helper/global, and removed the Z-index diagnostic sibling dump.
- Infra checklist gained two required patterns: ARM64 disasm probe and auto-revoke `Loaded`.

## 2026-06-18 — VDS and Taskbar Folder Menus submission

### Virtual Desktop Switcher → v1.6 (new mod ID `taskbar-vd-switcher`, PR #4484)

- Discovered PR #3932 (`taskbar-vd-switcher`) was merged 2026-06-17 at v1.5.
- Decided to resubmit under cleaner ID `taskbar-vd-switcher` / "Virtual Desktop Switcher".
- Left a deprecation request comment on #3932 asking m417z to disable `taskbar-vd-switcher` once #4484 is reviewed.
- v1.6 changes vs merged v1.5:
  - YAML setting names updated to shared rubric canonical names (`gridRows`/`gridColumns`, `activeBackgroundColor`/`inactiveBackgroundColor`, `opacity`, `groupPaddingLeft`/`groupPaddingRight`, `groupOffsetY`) with `AliasedStr`/`AliasedInt` backward-compat lambdas; old names still work
  - "Master button" renamed to "Task View button" throughout YAML `$name` fields and README
  - `contentOffsetX/Y` removed (was a rubric misapplication; not applicable to text buttons)
  - Full settings table added to both in-mod readme and folder README
  - README screenshots expanded with full gallery

### Taskbar Folder Menus → v0.5 (new mod, PR #4485)

- New features added this session: right-click Shell context menu via `IContextMenu`, "Open in Explorer" header at top of each subfolder popup, default hover color `#4488FF`
- `WM_MENURBUTTONDOWN` define added (`#ifndef` guard for Windhawk's Clang environment)
- `contentOffsetX/Y` removed (rubric misapplication)
- `TextBlock.Foreground` bug removed (was preventing hover color inheritance from VSM resource dict)
- Renamed `SYMBOL_HOOK hooks[]` → `taskbarDllHooks[]` (CI validation requirement)
- Screenshots incorporated into both in-mod readme and folder README
- Full settings table in both readmes

### Fork / CI
- Synced fork main with upstream (208 commits)
- Both PR branches had stray `vertical-omnibutton.wh.cpp` (pulled from fork's diverged main); removed in follow-up commits
- Both PRs passed CI after fixes

## 2026-06-21 — PR status audit and VDS rename

- Audited all open PRs and maintainer comments across ramensoftware/windhawk-mods and m417z/my-windhawk-mods.
- Clock spacer confirmed stalled: PR #4443 (standalone) and PR #68 (integration) both at impasse; m417z wants a TextAlignment=Justify + non-justifying-spaces approach we couldn't get working; the generated Grid fallback was rejected for complexity. Research left at `taskbar-clock-customization-spacer/` and `_archive/clock-spacer/`. No further action planned.
- Discovered PR #4484 (virtual-desktop-switcher) was a mistake: m417z confirmed mod IDs are permanent; the mod is already published as `taskbar-vd-switcher`.
- Renamed local folder and source: `virtual-desktop-switcher/` → `taskbar-vd-switcher/`, `virtual-desktop-switcher.wh.cpp` → `taskbar-vd-switcher.wh.cpp`. Updated `@id`, `@name`, all asset URLs, and all references across CLAUDE.md, README.md, working_notes.md, work_log.md, research docs.
- Closed PR #4484; opened PR #4516 as clean single-file update to existing mod: https://github.com/ramensoftware/windhawk-mods/pull/4516
- Contacted m417z on PR #3859 to propose retiring vertical-omnibutton in favor of omnibutton-customizer (full superset: any grid, per-element nudge, item reorder). Awaiting response.

## 2026-07-16/17 — Notes reorg + VD switcher highlight fix, accent default, multi-monitor toggle

### Lab housekeeping
- `_claude_notes/` retired in favor of `.agents/` (README = durable rules incl. standing directives, working_notes = current state only, work_log, knowledge/, tools/, outputs/). `CLAUDE.md` and new `AGENTS.md` are identical thin pointers. Root README layout updated. `generate_folder_map.py` re-pathed for its new home.

### VD Switcher (all in-tree, v1.7)
- **Highlight bug root-caused and fixed**: lightweight-styling resources (`ButtonBackground` etc.) resolve once at template application; swapping them on a live button does nothing, so the startup-active button kept its baked-in highlight ("both buttons highlighted"). Removed the in-place `UpdateHighlights` path entirely; every VD notification now does a full grid rebuild (`RebuildButtonGrid`). **Live-tested OK.**
- `activeColor` default changed to `"accent"`; empty now means native surface (matching all other color settings). Critical tokens quoted in all setting descriptions.
- **Experimental `multiMonitor` toggle** (issue #4785 follow-up comment by Deen-0x): secondary taskbars discovered from IconView `XamlRoot()`s (GetTaskbarXamlRoot is primary-only), registry of secondary tray grids persists across toggle flips, injection via shared `InsertGridIntoTrayColumns` helper, rebuilt alongside primary on every notification, pruned on monitor disconnect. Tray positions only; may need Explorer restart after enabling. **NOT yet tested** — testing deferred (Explorer restarts break the user's desktop order/workflow).
- GitHub recon: no PR/issue ever asked for multi-monitor except the #4785 comment; issue #4784 is the hover/hit-test report (full-rebuild change may help it — verify during testing); m417z's pending ask on PR #4516 is a changelog update.

## 2026-07-17 — Folder-menus crash fix, color generics rubric, both PRs advanced

- **Folder-menus crash-on-disable root-caused via minidump** (Windows.UI.Xaml.dll AV on a UI tick after mod DLL unload): XAML defers removed-subtree teardown; Click delegates/tooltips/boxed Content pointed into the unloaded DLL. Fix mirrors taskbar-ai-quota: `ClearButtonEventState()` revokes/clears before removal. Live-tested OK (race — log window open masks it).
- **Color rubric decided**: 5 canonical slots + optional identity axis; 4 generics `accent`/`accentLight`/`accentDark`/`transparent`, numbered shades silent; defaults never hardcoded hex. Applied to templates + both mods; VDS settings YAML reordered to canonical slot order; folder READMEs regenerated (verify-readme-sync passes).
- **PR #4516**: changelog blocker fixed, then v1.7 pushed superseding v1.6 (full-rebuild highlight fix, accent default + generics, multi-monitor). All CI green. Awaiting merge. Verified PR branch matches lab file byte-for-byte.
- **PR #4485**: folder-menus v0.6 pushed (all 4 review items + crash fix + generics + hover default `accent`); review reply posted. CI failed once on new "Mod authorship" PR-body requirement — section added, all CI green. Verified PR branch matches lab file byte-for-byte.
- **Template audit of remaining mods** (see dated section in `_templates/six-mod-settings-audit.md`): omnibutton needs Color-returning generic parser; privacy-anchor needs tooltip-clear before removal (folder-menus crash class), UI-thread revoker clear, and `activeColor`/`slashColor` shape fixes; tray-utility conformant. Added teardown contract to lifecycle template + `_templates/compile-check.ps1`.
- Both mods live-tested locally (VDS v1.7 incl. accent default; multi-monitor toggle behavior per session notes).
- Compile-check workflow established: Windhawk's bundled clang with `-fsyntax-only -DUNICODE -D_UNICODE -include windows.h -include windhawk_api.h`.
- **Readme/screenshot sync audit** (`_templates/verify-readme-sync.ps1`): vd-switcher MATCH (screenshots flagged for post-v1.7 refresh); folder-menus MATCH; omnibutton MISMATCH fixed by regenerating folder README from embedded (screenshots still vertical-era); privacy-anchor MISMATCH by design (unify before submission); taskmanager-tail MISMATCH (needs user decision); tray-utility MISMATCH (folder README richer — needs a pass); clock-spacer MISMATCH (stalemate, no action); system-tray-grid-lines unscannable (no .wh.cpp).

## 2026-07-17 — OmniButton + privacy-anchor fix round and template uniformity pass

All compile-checked; live testing FAILED — see the next entry.

### OmniButton Customizer
- Applied the template-audit items: generic color tokens in `ParseHexColor` (accent family + transparent, hex-digit validation, bare hex kept for back-compat); folder README regenerated; dev checklist moved to `knowledge/omnibutton-test-checklist.md`.
- Test round 1 findings (dual-height): smart grid never right (old `ResolveGeometry` naive division + min-2-rows clamp + default cols 2 meant auto never engaged); disable-with-percent strands "80%".
- Fixes attempted: `ResolveGeometry` rewrite (single column when items fit taskbar height); `ClearTransformsRecursive` cleanup sweep for the stranded percent.
- Test round 2 finding: percent vanished after toggling the Windows battery-percent switch. Attempted fix: deferred `LayoutUpdated` re-scan extended to watch for a late percent TextBlock; coupled-mode battery CP pinned `HorizontalContentAlignment(Left)`.
- **Animations removed by user direction** ("I wanted something different — no animations"): `*ColorTo` settings, `colorAnimateDuration`, all Storyboard machinery stripped; archived at `knowledge/omnibutton-color-animation-archived.md` (recovery: `git show 2f6ec2c:...`). The feature had been added by an earlier agent session in 4e5e347 without a design conversation.
- **Template adoption**: grid math replaced by verbatim `_templates/smart-grid-layout.h` copy (`windhawk_mod_templates::smart_grid`); `GetCell` replaces `ItemGridPos`+`ShortGroupCenterNudge`; GridMode derived from gridRows/gridColumns (0/0 = AutoSmart). Added `shortGroupPosition`. Settings block reordered to canonical profile order; `fillOrder` option value now `columnFirst`. README_MATCH.

### Privacy Indicator Anchor
- Applied all four template-audit fixes (tooltip/automation-name clear in `RemoveSyntheticIcons`; revoker clear on UI thread; `activeColor` string; `slashColor` canonical parser). `copilot` added to default `itemOrder`; auto columns default.
- Test round 2 finding: hardware camera/mic/location activity never lights icons — the mod only mirrored native tray glyphs, which Windows never shows for hardware-LED cameras. Copilot slash-then-clear identified as the disabled→installed initial-poll transition. Slash geometry verified byte-identical across all history (no direction change in code).
- **Added ConsentStore usage-record in-use detection**: `CheckCapabilityInUse(webcam/microphone/location)` scans `CapabilityAccessManager\ConsentStore` (packaged + NonPackaged, HKCU + HKLM) for `LastUsedTimeStart` set with `LastUsedTimeStop == 0`; polled every 3s, ORed with glyph-driven state.
- **Template adoption**: same verbatim smart-grid block as omnibutton (`ComputeIconPlacement` deleted); `gridRows` added; enums parsed at load (`colFirst` legacy spelling accepted); settings reordered to canonical profile order.

## 2026-07-17 — FAILED live-test round: privacy-anchor + omnibutton fixes did not work

User verdict after compiling the fresh versions (battery percent on): **"All the
problems persist. No work has been accomplished."** Recorded per user direction.

- **Privacy anchor — camera still not detected.** The user has a hardware
  camera kill switch; the ConsentStore usage-record detection
  (`LastUsedTimeStop == 0`) did not light the camera icon, and/or the
  disabled-state slash did not reflect the kill switch. The whole
  camera-detection stack (DeviceAccess consent, SetupDi present-device probe,
  ConsentStore) remains unproven against this hardware.
- **Privacy anchor — copilot slash still appears then disappears**, on a
  machine where Copilot does not exist and is disabled in every way including
  group policy. `CheckCopilotInstalled` is returning a false positive (likely a
  stale WebExperience package registration whose path still exists), and
  `CheckCopilotDisabled` only checks `ShowCopilotButton` — it does NOT check
  the Copilot group policy (`Software\Policies\Microsoft\Windows\WindowsCopilot`
  `TurnOffWindowsCopilot`) or other disable mechanisms. Known gap, unfixed.
- **OmniButton — still a mess.** Battery percent problems persist despite the
  late-percent LayoutUpdated fix, and switching `batteryPercentMode` to
  independent makes the battery glyph disappear entirely (worse than coupled).
  The static-analysis root cause for the missing percent was evidently wrong or
  incomplete; the independent-mode transform math has never been verified live.
- The smart-grid template adoption (both mods) compiled and is not implicated
  by the user's testing ("if we actually have the good gridding we can trust it
  for now, something else is going wrong") — but it is also UNTESTED.

Lesson recorded: two rounds of compile-checked static fixes have now failed
live testing. The next session must start from live evidence — Windhawk logs
from the user's machine (`[Battery]`, `[Layout]`, `[Cam]`, `[Copilot]`,
`[Usage]` lines) — before writing any more code.

## 2026-07-17 — Agent-files audit and notes-system restoration

User found the notes system adulterated: working_notes had grown to 244 lines
of past-tense completed work, stale feature descriptions (animated colors,
deleted `ComputeIconPlacement`), resolved PR items, and reference material.
Restored the intended flow (pointers → .agents/README → working_notes as a
lean living todo list, completed work in this log):

- Verified `CLAUDE.md` and `AGENTS.md` byte-identical pointers (they were).
- Re-sorted this work log into chronological order (entries had been appended
  out of sequence: 05-05 after 06-17, 06-15/06-21 after 07-17).
- Moved durable/reference material out of working_notes into knowledge/:
  `mod-infrastructure-checklist.md`, `pr-review-comments.md`,
  `privacy-anchor-design-notes.md`.
- Moved this session's completed work descriptions from working_notes into the
  session entry above; rewrote working_notes as a short per-mod todo list.

## 2026-07-17 — Privacy anchor: copilot false positive root-caused with live evidence

First evidence-driven fix round (probed the user's actual registry from the
session shell instead of guessing):

- **Proven**: `CheckCopilotInstalled` matched `MicrosoftWindows.Client.WebExperience_`
  — that package is the **Widgets host**, registered on this machine at a valid
  `C:\Program Files\WindowsApps\...` path, while no `Microsoft.Copilot_*` /
  `Microsoft.Windows.Ai.Copilot_*` package exists. That is exactly the
  installed=true false positive that cleared the copilot slash on a
  Copilot-free machine. Fix: WebExperience prefix removed from the match list.
- `TurnOffWindowsCopilot` group-policy check added to `CheckCopilotDisabled`
  (HKCU + HKLM `Software\Policies\Microsoft\Windows\WindowsCopilot`). Probe
  showed neither policy key nor `ShowCopilotButton` exists on this machine, so
  "no Copilot package" is now the operative (and correct) disable signal.
- Camera ground truth captured: `Integrated Camera` (USB\VID_174F) currently
  `Status=OK`; ConsentStore webcam layout matches the usage scanner exactly
  (packaged apps as direct children, Brave/Zoom under `NonPackaged`, all with
  `LastUsedTimeStop != 0` while idle). Open question for the user's next test:
  whether the hardware kill switch removes/errors the PnP device (detectable)
  or is a software-invisible shutter (not detectable; usage-lighting only).
- Embedded readme copilot wording corrected (no longer claims the Web
  Experience Pack is Copilot). Compile-checked OK; awaiting live test.

## 2026-07-18 — Privacy anchor: copilot/slash/mic confirmed; camera kill switch proven undetectable

User test round 4 results and the probe chain that settled the camera question:

- **Confirmed working by user**: copilot slash now sticks (WebExperience
  false-positive fix held); native active-mic icon suppression works with the
  mod's mirrored icon showing correct state; camera in-use lighting fires when
  actually streaming.
- **Slash direction**: default changed `rising` → `falling` (`\`), per user
  intent (chosen to avoid colliding with the glyphs); options reordered.
- **Camera kill-switch probe chain (all live on the user's Legion)**:
  (1) switch active during test → PnP `Status=OK`/`CM_PROB_NONE`, consent
  `Allow` both hives, no Lenovo privacy registry keys; (2) MediaCapture
  InitializeAsync SUCCEEDS with the switch active — the switch cuts the sensor
  stream only; (3) successful init writes NO ConsentStore usage record —
  records are written on streaming, not open (Brave/Zoom entries confirm real
  usage does); mid-stream kills don't dim the icon because the app's record
  keeps Stop=0; (4) `LENOVO_GAMEZONE_DATA` exists but is access-denied
  unelevated; a guarded WMI `GetCameraStatus` check was added to the mod
  (compile-checked) — then the user's ELEVATED probe showed this firmware has
  NO camera/privacy methods at all (thermal/fan/GPU/keyboard/OC only).
- **Verdict**: the kill switch is invisible to software at every level;
  hardware-disabled detection is impossible on this machine. Camera icon
  contract: bright = actively streaming. The Lenovo WMI check should be gated
  or removed next session; readmes must document the limitation.
- Probe tooling kept at `.agents/tools/camera-probe.ps1`.
- User directive recorded: replace the 3-second polling with event-driven
  detection (RegNotifyChangeKeyValue + WMI events) once detection stabilizes.
