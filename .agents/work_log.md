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

## 2026-07-18 — Taskbar Folder Menus PR #4485 revision pass

Implemented the requested PR corrections and the first repair for the user's
post-resume hidden-icons overlap in the lab checkout (not yet copied to the PR
branch):

- Replaced the invalid private menu-message constant with the documented
  `WM_MENURBUTTONUP` path and made nested Shell context menus use
  `TPM_RECURSE`.
- Added `IContextMenu3`/`IContextMenu2` message forwarding for dynamic,
  owner-drawn, and keyboard-driven classic Shell menu extensions, with
  `CMINVOKECOMMANDINFOEX` invocation and Shift extended verbs.
- Added `Open in Explorer` to the configured root popup as well as lazy folder
  submenus.
- Applied `[[clang::no_destroy]]` to the namespace-scope strong XAML holders,
  removed the unused strong parent reference, removed the unused version
  include/library, rejected off-thread menu subclassing, and made automatic row
  capacity DPI-correct.
- Reworked taskbar-start retries to validate the live `SystemTrayFrameGrid`
  instead of trusting a non-null cached Grid. Detached grids are released on
  the UI thread, native children recreated into the reserved folder column are
  shifted back out, and named placement anchors no longer fall back to column
  zero while the tray is still loading.
- Updated both detailed README copies and the root catalog. Windhawk syntax
  compile, embedded/folder README parity, and `git diff --check` all pass.
  Live right-click, unload, and overnight sleep/wake tests remain required.
- First live right-click test then produced no action at all. Re-reading the
  archived working prototype exposed the regression: it monitored the popup
  menu's modal input loop with a thread-local `WH_MSGFILTER`, while the Shell
  rewrite had replaced that path with only a subclass on `Shell_TrayWnd`.
  Restored the message-filter hook, added recursive `MenuItemFromPoint` hit
  testing for root and nested popups, and kept the window subclass for
  `IContextMenu2`/`IContextMenu3` extension-message forwarding. Added explicit
  logs for right-button capture, menu-item lookup, PIDL binding, context-menu
  creation, and command invocation. Compile/readme/diff checks still pass;
  this second right-click path awaits live retest.
- Second live test confirmed the classic context menu and ordinary commands,
  but exposed modal-loop reentrancy: Properties could be delayed or batched,
  while the taskbar button remained highlighted. The Shell context menu was
  still being tracked and invoked from inside the original folder popup's
  `WH_MSGFILTER` callback. Changed right-click to clone/queue the selected PIDL,
  call `EndMenu`, let `TrackPopupMenu` fully unwind, destroy/free the folder
  popup, and only then create the Shell context menu. Kept the owner subclass
  through that second menu for extension forwarding, stopped forcing either
  asynchronous or synchronous invocation semantics, and posted `WM_NULL` after
  each tracked popup to complete menu dismissal. Static validation still
  passes; Properties/modal-verb behavior awaits the next live test.
- Third live test confirmed that fully unwinding the folder popup fixed
  Properties invocation, but also confirmed the usability cost: the original
  folder popup vanished as soon as an item was right-clicked. Reworked this
  into a hybrid nested flow. Dismissing the Shell context menu now resumes the
  still-active folder popup, allowing a mistaken target to be retried. A
  positive Shell command retains its `IContextMenu`, calls `EndMenu`, waits for
  the folder popup to unwind and be destroyed, then invokes the queued verb
  outside the modal menu loop. Compile, README parity, and diff checks pass;
  the cancel/retry and positive-command branches await live testing.
- Fourth live test showed the nested attempt still collapsed the original
  popup immediately. The nesting call was being made too early, from the raw
  `WH_MSGFILTER` `WM_RBUTTONUP` callback. Corrected the event sequence to match
  the Win32 menu contract: the filter now hit-tests and posts
  `WM_MENURBUTTONUP` to the menu owner, returns from the raw input callback,
  and only then enters the Shell `TrackPopupMenuEx(..., TPM_RECURSE)` call from
  the owner subclass. Static validation passes; whether this preserves the
  original popup on Shell-menu dismissal awaits live testing.
- Fifth live test confirmed the corrected owner-message sequence: the original
  folder cascade stays available, context-sensitive Shell menus are correct,
  and Properties/other commands behave normally. One cancellation edge
  remained: a click outside the nested Shell menu was consumed by that menu,
  so the resumed outer popup kept the XAML button highlighted. Added nested
  menu hit-testing that distinguishes Escape or a click back into the folder
  cascade from a click outside both menu trees; only the latter now calls
  `EndMenu` to close the outer popup and release the button state. Static
  validation passes; this final click-away branch awaits live confirmation.
- Sixth live test screenshot showed that the remaining highlight occurred on
  the positive Properties path, not ordinary context-menu cancellation. The
  Properties handler blocks inside `IContextMenu::InvokeCommand`, which was
  still called before the original XAML `Button.Click` callback returned.
  Changed queued Shell verbs to be dispatched through a registered message on
  the taskbar owner after `ShowFolderMenu` has destroyed the popup and returned;
  the temporary owner subclass removes itself before invoking the verb. This
  lets the XAML pressed visual clear before a modal Properties dialog begins.
  Static validation passes; the modal-verb highlight fix awaits live testing.
- Seventh live clarification established that Properties was incidental: any
  folder popup dismissed by clicking away left the button highlighted until
  the pointer returned to the taskbar. The native `TrackPopupMenu` input loop
  can prevent XAML from receiving its pointer-exit transition, leaving the
  Button in a stale `PointerOver` state. The click handler now weak-captures its
  Button and explicitly returns it to the `Normal` visual state after the menu
  loop exits. Compile, README-sync, and diff checks pass; live confirmation is
  pending.
- Eighth live test showed that selecting `Normal` alone was insufficient: the
  Button immediately restored `PointerOver` from its stale internal input
  state, and even the Windows key did not clear it. The reset now releases any
  pointer captures, removes the Button from hit testing, collapses it through a
  synchronous layout pass, restores it, and only then selects `Normal`. This
  invalidates the stale pointer target rather than changing only its appearance.
  Compile, README-sync, and diff checks pass; live confirmation is pending.
- Ninth live test confirmed the input-state reset: folder menus, context menus,
  Properties, click-away dismissal, and button highlight recovery all look
  correct. Final defaults were changed to Smart automatic layout,
  `gridColumns: 0` (automatic Smart column cap), and 24 px button width. The
  loader now preserves zero for the Smart-layout sentinel while Fixed grid
  still enforces at least one column. Source compilation, embedded/folder
  README parity, and `git diff --check` pass. The mod is ready for fresh
  screenshots.

## 2026-07-18 — Privacy Indicator Anchor camera-switch reorientation

Revisited the prior "impossible" camera-switch conclusion against Microsoft's
camera privacy-control contract. The earlier live probes remain valid evidence
that this Legion exposes no useful PnP, ConsentStore, Lenovo registry, or
Lenovo WMI state change, but they did not test the Windows 11
`CameraOcclusionInfo` API or the contents of the still-running camera stream.

- Microsoft's supported kill-switch design keeps the camera present and apps
  streaming while the ISP substitutes blank frames, which matches the user's
  observation and makes stream-based inference technically plausible.
- The new detector order is deterministic first: test
  `CameraOcclusionKind::CameraHardware` and `CameraOcclusionInfo.StateChanged`
  while streaming. Only if the driver does not expose that state should a
  separate `SharedReadOnly` frame-reader probe compare open/closed luminance,
  spatial variance, temporal variance, and fixed-frame characteristics.
- Expanded `.agents/tools/camera-probe.ps1` to initialize in
  `SharedReadOnly` mode and report camera-hardware occlusion support/state. The
  script passed a parser check, and Windows PowerShell 5.1 resolved both the
  existing MediaCapture type and the newer CameraOcclusion type. PowerShell 7
  cannot project that newer WinRT type here, so the script now exits with a
  clear host requirement. It was deliberately not run because doing so opens
  the camera; the next live step requires the user's opt-in and two runs,
  switch open and switch closed.
- Recorded the architecture and false-positive limits in
  `.agents/knowledge/privacy-anchor-design-notes.md`. No privacy mod source was
  changed in this reorientation pass.

## 2026-07-18 — Standard camera hardware-occlusion detector

The user ran the `SharedReadOnly` camera probe in both physical switch states.
The camera reported `CameraHardware` support in both runs. With the switch
blocked, `IsOccluded=True` and `IsOcclusionKind(CameraHardware)=True`; with the
switch enabled, both state values were false. This proves a direct standard
Windows signal on this camera without requiring black-frame inference.

- Replaced the speculative Lenovo `LENOVO_GAMEZONE_DATA` WMI check with a
  thread-owned `CameraPrivacyMonitor` using Windows 11
  `VideoDeviceController.CameraOcclusionInfo`.
- The monitor opens the default video device in `SharedReadOnly` mode without
  starting preview or frame capture, reads the initial state, subscribes to
  `StateChanged`, and wakes the existing state-refresh loop on transitions.
- Cleanup removes the event handler and closes `MediaCapture` before the state
  thread exits. Temporary initialization failures retry; a driver that reports
  no `CameraHardware` support falls back to existing software-policy, device,
  PnP, and ConsentStore checks without repeated camera opens.
- Updated the embedded and folder documentation to describe the portable
  driver-support boundary. The persistent event path still needs a live
  Windhawk switch test. The camera probe does not establish microphone state;
  microphone hardware/software characterization remains separate.
- Windhawk syntax compilation, PowerShell probe parsing, stale Lenovo/WMI
  source search, and `git diff --check` pass. The privacy folder README remains
  intentionally richer than the embedded README, so the existing planned
  README-unification check still reports a mismatch.

## 2026-07-18 — Privacy evidence tooltips and Settings actions

The user live-confirmed the first working camera slash: it responds immediately
to the Legion's physical camera switch. Follow-up review corrected the UI
semantics against Microsoft's API contract: an idle-camera
`CameraHardware` occlusion report is strong device evidence but is advisory,
not an absolute security guarantee.

- Replaced the single "Hardware disabled" tooltip label with reason-specific
  states for user/system access denial, policy, location service, endpoint
  mute, device disabled/unavailable, advisory camera occlusion, Copilot not
  installed, and the Copilot taskbar setting.
- Microphone privacy policy/ConsentStore checks are now distinct from default
  endpoint mute. A keyboard Fn mute therefore reports software/firmware-visible
  endpoint mute rather than claiming a physical hardware cutoff.
- Moved tooltips to transparent full-slot hit targets, fixing the Copilot
  Viewbox tooltip. Added click actions that choose documented `ms-settings:`
  pages according to the item and current reason.
- Added `cameraHardwareDetection` so users can disable the persistent
  `SharedReadOnly` controller monitor if their camera powers an LED, surfaces
  an indicator, or has compatibility trouble. No preview or frame capture is
  started.
- Fixed a pre-existing short-circuit state-update bug: all atomic state and
  reason exchanges now run on every refresh instead of stopping after the first
  changed field. Windhawk syntax compilation and `git diff --check` pass; the
  refined tooltips, clicks, and monitor toggle await live testing.

## 2026-07-18 — Event-driven camera monitor hardening

- Removed the camera controller's three-second `GetState` read. The existing
  `CameraOcclusionInfo.StateChanged` subscription is now the primary path, with
  a five-minute liveness read handled by the existing state worker rather than
  a second watchdog thread.
- Temporary camera initialization failures now back off through 10 seconds,
  30 seconds, 2 minutes, and 10 minutes. A definitive unsupported result is
  cached for the current enabled session instead of reopening the controller.
- Camera monitoring now requires both `cameraHardwareDetection` and the
  `camera` token in `itemOrder`; disabling either releases the event handler and
  `MediaCapture`, while re-enabling triggers a fresh initialization.
- Updated both camera-monitor documentation surfaces and settings diagnostics.
  The installed Windhawk Clang toolchain reports syntax success and
  `git diff --check` passes. Release/reopen and single-monitor behavior still
  require a live settings test.

## 2026-07-18 — Removed Privacy Anchor's three-second global sweep

- Split the monolithic privacy refresh into domain flags for location, mic,
  camera, each ConsentStore usage surface, Copilot installation/policy, and
  Copilot process activity. Native events now refresh only the affected domain.
- Added asynchronous `RegNotifyChangeKeyValue` subscriptions for each
  location/microphone/webcam ConsentStore tree plus location policy/service,
  Copilot policy/taskbar, and AppModel package state. Notifications re-arm
  after each signal; missing keys temporarily watch their parent, transient
  failures back off through 10 seconds, 30 seconds, 2 minutes, 10 minutes, and
  30 minutes, and access-denied watches remain disabled for the mod session.
- Added `DeviceAccessInformation.AccessChanged` subscriptions for microphone
  and camera access plus a video-capture `DeviceWatcher` for camera topology.
  Existing endpoint-volume and camera-occlusion subscriptions remain intact.
- Replaced the fixed 3000 ms worker timeout with a dynamic wait over stop,
  manual, registry, device, and hardware events. Only a targeted one-minute
  Copilot process check, five-minute full health reconciliation, camera
  watchdog, and failed-monitor retry deadlines remain timer-driven.
- Updated both user-facing monitoring descriptions. Windhawk syntax compilation
  and targeted `git diff --check` pass; live event, settings, teardown, and
  sparse-timer behavior still require testing in Explorer.

## 2026-07-18 — Privacy Anchor four-state visual emphasis

The user live-confirmed that the event-driven location, microphone, camera,
permission, and hardware-shutter paths now produce the expected states, with
occasional Windows/driver latency but no missing behavior observed.

- Made the four presentation states explicit: idle/available, active,
  disabled/unavailable, and active while disabled. The combined state now keeps
  active color/emphasis beneath the slash by default instead of collapsing into
  ordinary disabled styling; `alertWhenBlockedAndActive` can opt out.
- Replaced the enlarged duplicate-glyph “glow” with fixed-footprint concentric
  halos and three selectable treatments: steady, breathing pulse, or expanding
  radiation rings. Added independent glow color, opacity, reach, and cycle-time
  controls; Storyboards are stopped and released before XAML subtree teardown.
- Added independent idle and disabled colors plus disabled opacity. Existing
  active/slash colors remain composable, including alpha-bearing hex values.
- Unified the embedded and folder READMEs and documented the four states, visual
  controls, width-preserving effect, and a deliberately high-impact preset.
- Windhawk Clang syntax compilation, README parity, and targeted
  `git diff --check` pass. Live visual tuning and final screenshots remain.

## 2026-07-19 — Privacy Anchor settings-metadata parse fix

- Fixed Windhawk's YAML parse failure in the new `glowSpeed` description. The
  plain scalar contained `Range:`, which YAML treated as a nested mapping; it
  now uses a folded description without an ambiguous colon.
- Re-scanned all one-line setting descriptions for the same pattern. Windhawk
  Clang compilation, README parity, and targeted `git diff --check` pass.

## 2026-07-19 — WIP handoff to Clock Spacer debugging

- Closed this Privacy Indicator Anchor session without declaring the mod
  complete. Its event-driven state paths are substantially working and the
  latest source passes static checks, but visual-state tuning, tooltip/action
  tests, monitor lifecycle tests, and screenshots remain WIP.
- Promoted the local Taskbar Clock Customization Spacer to the next-chat focus.
  The handoff covers both the elastic-spacer/layout behavior and the repeating
  `PDH_INVALID_DATA` (`0xC0000BC6`) failures captured during privacy testing.
- Preserved the error evidence and investigation checklist in
  `.agents/knowledge/taskbar-clock-spacer-pdh-invalid-data.md`. No Clock Spacer
  source change was made during the privacy session; the next chat should
  reproduce and instrument the problem before attempting a fix.

## 2026-07-19 — Clock Spacer direction settled and standalone matured to v1.1

Reconciled the clock-spacer state against git and both PRs. The lab folder
`taskbar-clock-customization-spacer/` holds m417z's full clock mod with the
spacer grafted in (`@id taskbar-clock-customization`, v1.7.5) — the vehicle for
PR #68, not a deliverable of ours. The actual standalone mod
(`@id taskbar-clock-spacer`, v1.0, 802 lines) existed only on the PR #4443
branch and in `_archive/clock-spacer/`; the two copies were byte-identical.

- PR #68 is finished: the maintainer preferred a `TextAlignment::Justify`
  approach without generated layout elements and implemented it himself
  (2026-06-29). The user closed it out by comment on 2026-07-19. The standalone
  companion mod on PR #4443 is the surviving path.
- The PDH `0xC0000BC6` flood is out of scope for the deliverable: it originates
  in m417z's performance-metrics code. The standalone mod contains no PDH calls.
- Created `taskbar-clock-spacer/` and brought the standalone to v1.1.

Engine work backported from the integration copy, which had matured while the
standalone did not: in-place text updates guarded by a layout key (the old code
destroyed and rebuilt the whole generated subtree every clock tick), first/last
segment edge anchoring, `TextAlignment` propagation, insertion at the source
index, and `\r` handling in line splitting.

Defects fixed in the boilerplate: `GetTaskbarXamlRoot` had no null guards on the
hooked symbols, `taskBand`, or `iunk`, and tested the host shared_ptr with `&&`
instead of `||` — and `Wh_ModInit` logged "initial scan disabled" on symbol
failure without actually disabling the scan, so a resolution failure guaranteed
a null call. Added the `_M_ARM64` and `#else #error` arms, `[[clang::no_destroy]]`
on the state vector, a `FALSE` return from `Wh_ModInit` on hook failure, and
`TrayUI::StartTaskbar` as the primary wait-for-module hook (symbol verified
against `taskbar-ai-quota` and `taskbar-tray-show-on-hover`) with the
`LoadLibraryExW` watcher retained as fallback. Switched the deprecated
`Wh_SetFunctionHookT` to `WindhawkUtils::SetFunctionHook`.

Usability: added `minSpacerWidth` (default 0, no change to default rendering) and
a one-time log explaining the zero-slack condition. A tinted spacer-diagnostic
mode was built and then cut at the user's direction — reserving pixels to report
on missing pixels disrupts the layout it is reporting on, and it was scope creep
on a single-token mod. Both README layers rewritten to lead with the two hard
requirements (the companion dependency and the fixed clock width) and to put
troubleshooting ahead of settings.

Windhawk Clang syntax compilation passes. The mod is NOT live-tested: the
`TrayUI::StartTaskbar` path, the visual result, and teardown all still need a
run in Explorer. Branch `add-taskbar-clock-spacer` is ~248k deletions out of date
against upstream/main and needs rebasing before PR #4443 can merge.

## 2026-07-19 — Tray Utility Customizer v0.4: first full template adoption

Reworked tray-utility-customizer from its pre-template v0.3 into the lab's
first fully template-conformant mod. Adopted `_templates/smart-grid-layout.h`
v1.0 as a verbatim block (replacing the third independent inline grid
implementation) and `_templates/injected-grid-column.h` for the dedicated-column
lease. Extended that template to v1.1 with an `AcquireAt(column)` overload so a
mod can lease a column at a self-resolved index (borrowing the hidden-icons or
Emoji column for multi-column layouts); `Acquire(anchor)` now delegates to it.
No other adopters existed, so no rollout comparison was needed.

Settings converted to the canonical profile and order: `gridMode`
(autoSmart/singleRow/singleColumn/fixedRows/fixedColumns/fixedGrid),
`smartLayout`, `gridRows`, `gridColumns`, `fillOrder`, `shortGroupPosition`
(new), `shortGroupAlign`. Eliminated old junk: the `layoutMode` key (mod
unpublished, no alias kept), the v0.2 stale-layout recovery shims, and legacy
`overflowFirst`/`emojiFirst` itemOrder aliases. `detailedLogging` default
flipped to false. Fixed the code/docs contradiction about over-tall layouts:
docs now state that Smart automatic never exceeds the tray height while fixed
modes honor the requested shape (matching the code, which logs and honors).

Verification: Windhawk clang syntax check passes for the mod and the modified
template header; the smart-grid template test suite still passes;
`verify-readme-sync.ps1` reports README_MATCH. Root README updated (v0.4 row,
plus fixing the stale clock-spacer rows) and the six-mod audit matrix
re-audited for the Tray Utility column. NOT live-tested; screenshots pending.

## 2026-07-19 — Tray Utility Customizer v1.0 submitted (PR #4841)

Completed the same-day arc from template rework to submission. After the v0.4
template adoption, the session added experimental Left/Right-of-Start overlay
positions (self-correcting Start counter-shift), switched host placement from
render transforms to margin-based layout so flyouts anchor correctly, fixed
the after-Show-Desktop off-screen bug via injected-grid-column.h v1.2
(span-aware "after" anchors), and made the layout react to live utility
appearance/disappearance: effective-visibility IconView scanning in discovery,
per-host content baselines, visibility watchers, a throttled (250 ms) tray
LayoutUpdated intactness check, and a 150 ms debounced reapply. User
live-confirmed the emoji taskbar-settings toggle recenters the chevron and
re-gathers on re-enable, and captured the three README screenshots.

Submitted as PR #4841 on ramensoftware/windhawk-mods (branch
add-tray-utility-customizer, commit 4e95b3fe, @version 1.0 — bumped from the
user's 0.5 per new-mod convention). All three documentation layers verified
in sync (README_MATCH) and Windhawk clang syntax check passed at submission.

Post-submission: the upstream PR validation failed because the description
dropped the required "## Mod authorship" template section. Restored the full
template in the PR body (checked "The submitter, with AI assistance" and
"Claude"), pushed an empty commit to re-trigger CI (description edits don't
re-run the workflow), and all checks came back green: PR validation plus the
Windhawk 1.6.1 / 1.7.3 / 2.0.0-alpha.1 compile matrix. PR #4841 now awaits
maintainer review. A 250 ms tick-count throttle was also added to the tray
LayoutUpdated intactness check before submission so layout bursts collapse to
at most four checks per second.
