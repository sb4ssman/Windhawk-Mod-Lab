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

## 2026-07-19 — Folder Menus restart and Smart Grid live-confirmed

- Replaced the older inline folder-grid implementation with the verbatim
  `_templates/smart-grid-layout.h` block and canonical six-mode `gridMode`
  profile. Automatic row capacity now uses the live `SystemTrayFrameGrid` DIP
  height and the correct item-plus-spacing formula.
- Hardened startup with the standard three-module `IconView.Loaded` trigger,
  `TrayUI::StartTaskbar`, and a slow-sign-in retry fallback. The user
  live-confirmed that the layout looks correct and survives a restart.
- Updated the README gallery to use all six screenshots under
  `taskbar-folder-menus/assets/`: minimal two-button, destination tooltip,
  four-button horizontal and vertical layouts, Control Panel namespace, and
  whole-drive native-menu examples.
  Windhawk syntax compilation, embedded/folder README parity, local image-path
  checks, root catalog status, and `git diff --check` pass. PR #4485 was not
  changed.

## 2026-07-19 — Folder Menus v0.7 submitted to PR #4485

- Bumped the PR-ready version and init log from v0.6 to v0.7 for the canonical
  Smart Grid modes and startup-recovery behavior. Mirrored the user's final
  gallery-caption edit into the embedded README and reverified all three
  documentation layers.
- Rebuilt the PR head as a clean one-file commit on current upstream/main and
  safely replaced the stale fork history with force-with-lease. Updated the PR
  title/body and posted a reviewer-facing response covering both required
  review fixes, the DPI/layout improvement, and the live evidence for retaining
  `TrayUI::StartTaskbar` alongside `IconView.Loaded`.
- Initial PR validation requested a target-specific symbol-hook variable name.
  Renamed the generic `hooks` array to `systemTrayDllHooks`, amended the clean
  submission commit to `46f3301`, and retriggered CI. Final status is green:
  PR validation plus Windhawk 1.6.1, 1.7.3, and 2.0.0-alpha.1 compilation all
  pass.

## 2026-07-19 — Privacy restart-crash fix and submission gates

- Correlated repeated restart-time Explorer failures with Windows Error
  Reporting: `Windows.UI.Xaml.dll` access violation `0xc0000005` at stable
  offset `0x24c62f`, with Privacy Anchor v0.9 loaded. Root-caused destructible
  namespace-scope XAML/WinRT owners that could release after framework teardown
  because process shutdown does not guarantee `Wh_ModUninit`.
- Marked all Privacy Anchor XAML owners, revoker/state containers, and settings
  state `[[clang::no_destroy]]`. Controlled unload still clears them on the
  taskbar UI thread; the no-taskbar fallback now intentionally retains state
  instead of releasing it from Windhawk's unload thread. Windhawk syntax and
  the new exit-time-destructor audit pass. Version remains v0.9 pending live
  restart testing.
- Added lifecycle template v1.1 plus
  `_templates/exit-time-destructor-audit.ps1`. The six-mod adopter audit found
  the same unprotected-global class in VD Switcher, OmniButton, and Tray Utility;
  those require deliberate rollout before their next PR updates.
- Added `_templates/submission-preflight.ps1` and expanded the submission
  checklist. The automated gate now checks compilation, exit-time destructors,
  embedded/folder README parity, broken and unreferenced gallery assets,
  `git diff --check`, generic symbol-hook names, and the current upstream
  Windhawk validator. PR workflow checks now explicitly cover one-file branch
  scope, the intact authorship/AI section, current upstream base, the full CI
  matrix, and retriggering CI after description-only fixes.
- The real upstream validator caught and drove both Privacy hook arrays to
  target-specific names (`taskbarDllHooks`, `systemTrayDllHooks`); Privacy now
  passes upstream source validation. Its full preflight stops as designed on
  six unreferenced screenshots awaiting the later gallery pass.

## 2026-07-19 — Privacy restart failure: second startup crash root-caused

- The first post-fix restart still failed, but with a new signature:
  `KERNELBASE.dll`, exception `0x20474343`, offset `0xC1ADA`; the previous
  `Windows.UI.Xaml.dll` access violation did not recur.
- Parsed the fresh 14.6 MB Explorer minidump and mapped module-relative stack
  addresses through the installed Privacy DLL's symbol table. The actionable
  chain is `RunFromWindowThread` → `ApplyStyleOnWindowThread` →
  `ClearPrivacyStates` → `UpdateSyntheticState` → inlined
  `UpdateSyntheticOpacity` → `Application::Current` → `winrt::throw_hresult`.
  Early startup calls cleanup before verifying a live XAML root;
  `ClearPrivacyStates` unnecessarily repaints nonexistent synthetic UI, and
  the exception escapes the WH_CALLWNDPROC hook callback. The camera worker is
  absent from the failing chain.
- Template-conformance audit found Privacy only uses the Smart Grid algorithm
  verbatim and a color-parser variant. Its lifecycle is hand-rolled instead of
  adopting the guarded lifecycle template, its group-layout settings omit
  canonical `gridMode`/`smartLayout`, and its column lease is an older manual
  implementation that lacks injected-grid-column.h v1.2's span-aware
  after-anchor and unavailable-anchor behavior. No second source fix was made
  in this diagnostic pass.

## 2026-07-19 — Privacy crash fix, Start placement, and submission preflight

- Implemented the dump-driven restart fix and lifecycle-template v1.2:
  eliminated `Application::Current`, guarded cleanup/reapply on a live XAML
  root, stopped teardown from repainting absent synthetic UI, and contained
  exceptions in taskbar dispatch, native hooks, Loaded handlers, and property
  callbacks. The user confirmed this build survived a full system restart.
- Replaced remaining local infrastructure with applicable verbatim templates:
  Smart Grid v1.0, injected-grid-column v1.2, canonical settings profiles, and
  the lifecycle dispatcher. Added `_templates/start-placement.h` v1.0 by
  extracting the proven Tray Utility/VD Switcher pattern into a reversible
  owned-group lease, then copied it verbatim into Privacy Anchor for new
  experimental `leftOfStart` and `rightOfStart` settings.
- Examined and documented every PNG in `privacy-indicator-anchor/assets`: idle,
  horizontal and compact-grid disabled states, two active treatments,
  blocked-active treatment, and the four location/microphone/camera/Copilot
  evidence tooltips. Folder and embedded README copies remain synchronized.
- Mod compilation, StartPlacement header compilation, exit-time-destructor
  audit, exact template-block comparison, README parity, image inventory,
  `git diff --check`, and the networked upstream Windhawk validator all pass.
  Final readiness now depends on live testing the two new positions and the
  concise teardown/action/monitor checklist; version remains v0.9 meanwhile.

## 2026-07-19 — Privacy Anchor screenshots, start-placement v1.1, v1.0 submission (PR #4843)

- Screenshot audit found the four tooltip PNGs referenced but never committed
  (raw GitHub URLs 404ed); the user committed and pushed all ten. All images
  visually verified against their captions; the red-highlight caption was
  corrected to name the blocked-while-active camera.
- Pruned both README layers for submission: removed `## Files`, `## Status`,
  and the `privacy-diag.ps1` note; folded the experimental camera/Copilot
  sentence into `## Notes`. Layers verified byte-identical modulo raw URLs.
- Diagnosed the broken `leftOfStart`/`rightOfStart` geometry: v1.0 of
  `_templates/start-placement.h` pinned Start to a stale absolute X captured
  at Acquire and placed the Left group at the taskbar's left edge. Wrote
  template v1.1: group positioned relative to Start's live layout X each
  layout pass; constant Start counter-shift selected by visual-tree
  containment of Start in TaskbarFrameRepeater. Copied verbatim into the mod.
  Tray-utility-customizer still carries the pre-template inline copy with the
  v1.0 defects (noted for its next approved PR update). v1.1 is UNTESTED live.
- With explicit user approval, bumped @version 0.9 → 1.0 and submitted PR
  #4843 (branch add-tray-privacy-indicator-anchor, based on upstream/main
  7babeba1) with the TUC-style authorship body. Root README row updated.
- Post-submission reconciliation: PR #4843 CI green (validation + all three
  compile matrix jobs). PR states verified live: #4843, #4841, #4485 all open
  with no maintainer response pending on our side; m417z/my-windhawk-mods PR
  #68 remains open and still needs an explicit close. Cleared two stale note
  lines (TUC lab bump already committed; folder-menus smart grid gap resolved
  at v0.7).

## 2026-07-19 — Clock Spacer width-ratchet diagnosis and fix; taskmanager-tail README unified

- User corrected the record: the Start-adjacent positions in the submitted
  Privacy Anchor build were live-tested by them before approval; the
  "untested v1.1" caveat was struck from the notes.
- taskmanager-tail folder README rewritten to mirror the published embedded
  readme exactly (verified byte-identical), resolving the standalone-repo
  style divergence.
- Clock Spacer "multiplying spaces" diagnosed against the current TCC source
  (.tmp-windhawk-mods snapshot): (1) EffectiveLineWidth's ActualWidth fallback
  hard-set generated-row Width from a live measurement each tick — a monotonic
  width ratchet; (2) with the mod's maxWidth at 0, ApplyWidthConstraint on the
  parent cleared the StackPanel MaxWidth that TCC sets for its Max width
  feature. Fixed by never mutating the parent panel, collapsing the source
  block to zero width, and sourcing width only from settings or TCC's constant
  parent MaxWidth. maxWidth setting docs synced across all three layers.
- Established a local Windhawk clang syntax-check recipe (Windhawk's bundled
  toolchain, -DWH_MOD etc.) — recorded in knowledge/lab-local-compile-check.md;
  the fixed Clock Spacer compiles clean with it.

## 2026-07-19 — Clock Spacer rounds 2-3: weather {spacer}, width pinning; user-confirmed

- Round 2: restored the weather spacer from the archived integration design as
  a standalone-compatible `{spacer}` split token (wttr.in consumes %s as
  sunset; a literal {spacer} echoes through verbatim), and replaced explicit
  generated-row Width with stretch geometry.
- Round 3 (user screenshot showed lines exceeding TCC's 150px Max width): a
  StackPanel arranges a child at max(slot, desired), so the unspaced weather
  row dragged the uncapped generated panel past TCC's cap. The generated panel
  is now pinned to exactly the effective width (MinWidth + MaxWidth, constants
  only), and every row gets MaxWidth = width so over-long unspaced lines clip
  like the native block. Row caps reapply on the per-tick fast path. Also
  corrected the readme double-spacer table example per user wording.
- User live-tested and approved ("everything is looking good") — submission
  of the PR #4443 update authorized.
- With user approval after their live test, submitted the update to PR #4443:
  rebuilt branch add-taskbar-clock-spacer as one fresh v1.1 commit on current
  upstream/main (85a971d5), refreshed the PR title/body (changelog, companion
  framing, authorship section). Validator demanded module-indicating
  symbol-hook variable names — renamed to systemTrayDllHooks and used the
  "// taskbar.dll" comment form for the StartTaskbar hook. All CI green:
  validation + 1.6.1/1.7.3/2.0.0-alpha.1 compile matrix.
- Recorded the durable submission-preflight rules in .agents/README.md:
  module-named symbol-hook variables (or module comment above), the required
  "## Mod authorship" PR-body section and its push-time evaluation, the local
  Windhawk clang syntax check, and the established PR body format.

## 2026-07-19 — VD Switcher Issue #4830 candidate implemented

- Made the published `dot` label mode customizable through independent active
  and inactive indicator strings while preserving `●`/`○` as its defaults.
  Added independent indicator and Task View font-family settings; empty keeps
  the native font.
- Replaced desktop `Button` controls with `ToggleButton` controls whose checked
  value follows the actual current desktop. The native `Checked`,
  `CheckedPointerOver`, and `CheckedPressed` states now exist for Taskbar
  Styler, while click handling immediately restores the registry-backed state
  until the asynchronous desktop switch notification triggers a rebuild.
- Hardened the existing full-rebuild path: click tokens, tooltips, and boxed
  content are now owned per grid and cleared before XAML subtree removal.
  Adopted the process-shutdown lifetime contract for all settings/XAML owners,
  removed off-thread teardown fallback, guarded settings/unload on a live XAML
  root, and contained UI/native-callback exceptions.
- Updated both README layers with the new settings and the exact Taskbar Styler
  target. Windhawk compilation, exit-time-destructor audit, settings YAML
  parsing, README parity, and `git diff --check` pass. Version remains v1.7;
  live visual, switching, Styler-state, disable, and restart tests remain.

## 2026-07-19 — VD Switcher v1.8 submitted (PR #4844)

- User live-confirmed the Issue #4830 build and explicitly approved submission
  with the existing gallery. Bumped metadata/init logging from v1.7 to v1.8.
- Archived six legacy unreferenced screenshots under the mod's existing
  `Archive/assets/`; every image remaining in the published `assets/` folder is
  referenced by both README layers.
- Full local submission preflight passes: Windhawk compilation, exit-time
  destructor audit, README parity and image inventory, `git diff --check`, and
  the current networked upstream validator.
- Submitted a clean one-file update on branch
  `update-taskbar-vd-switcher-v1.8`, commit `0d58a4a6`, based on current
  upstream/main `7babeba1`. PR #4844 is ready for review and mergeable. Final
  CI is green: PR validation plus Windhawk 1.6.1, 1.7.3, and
  2.0.0-alpha.1 compatibility builds all pass.

## 2026-07-19 — OmniButton template-conformance audit

- Reconciled the current v1.0 source against every lab template. The Smart Grid
  implementation body exactly matches `_templates/smart-grid-layout.h` v1.0
  (its `<algorithm>` include is hoisted earlier in the single-file source), the
  settings profile is canonical for the mod's capabilities, and the generic
  color-token behavior is present through the required Color-returning parser.
  README parity and Windhawk syntax compilation pass.
- Confirmed that button-surface, injected-column, Start-placement, and the
  future placement contract do not apply to the current native-OmniButton
  design.
- Found one substantive applicable-template gap: XAML lifecycle v1.2 has not
  been adopted. The exit-time destructor audit reports 14 namespace-scope
  XAML/WinRT owners, while the hand-rolled dispatcher and unload/settings paths
  lack the template's full live-root, exception-containment, UI-thread cleanup,
  retention, and retry guarantees. Both symbol-hook arrays also remain generic
  `hooks` variables and must be renamed or annotated before upstream validation.
- Kept the functional diagnosis evidence-driven: the last live test still has
  an incorrect coupled battery percentage and a missing glyph in independent
  mode. The next diagnostic input is the mod's `[Battery]` and `[Layout]` logs;
  no code fix was made during this audit.

## 2026-07-19 — OmniButton full-template test candidate

- Completed the post-audit hardening pass while preserving the mod's defining
  scope: granular control of every native OmniButton item. Added canonical
  `gridMode` and `smartLayout`, retained the exact Smart Grid v1.0 body, and
  exposed per-item color, size, font family, opacity, visibility/order, and X/Y
  offsets for wifi, volume, battery, and percentage.
- Fixed the broken `itemOrder` contract: omitted tokens are now actually hidden
  instead of silently appended. Coupled mode keeps battery/percentage together
  as one native grid item with independent styling/nudges; independent mode
  gives each child a real slot within a full-grid inner-panel footprint.
  Collapsed-sibling natural offsets are accounted for explicitly.
- Replaced guessed cleanup defaults with a dependency-property lease: every
  changed native XAML property snapshots its exact original local value and is
  restored in reverse order. Adopted lifecycle v1.2 behavior throughout:
  `no_destroy` XAML owners, UI-thread-only cleanup/intentional retention,
  live-root settings guards, exception containment, retry recovery, and
  `TrayUI::StartTaskbar`. Both symbol-hook arrays now identify their modules.
- Removed the superseded rediscovery/blanket-ClearValue cleanup path and added
  battery/layout diagnostics reporting native slots, inner-panel structure,
  visibility, and resolved coordinates for the live test.
- Verification passes: Windhawk Clang compilation, exit-time destructor audit,
  settings YAML parsing (36 settings), exact Smart Grid body comparison, README
  parity, `git diff --check`, and the current upstream Windhawk validator via a
  clean source-only staging copy. Full submission preflight intentionally still
  stops on the three vertical-era screenshots; replace them after live testing.

## 2026-07-20 — OmniButton restoration, centering, and safe placement candidate

- User screenshots from the first full-template candidate showed that disabling
  did not fully restore native battery-percentage alignment and that a forced
  2×2 layout was visually off-center on a double-height taskbar.
- Removed all outer `ControlCenterButton` width/height/alignment mutations. The
  mod now changes only the internal items host, restores leased properties, then
  invalidates/updates the native button and parent layout during cleanup.
- Made independent battery/percentage the default. Native battery/percentage
  children are measured at their desired size and receive computed horizontal/
  vertical center offsets; presenters continue to center their content through
  native content alignment. Per-item nudges remain available for optical
  correction rather than basic cell geometry.
- Replaced the old button-padding setting with the canonical four group-padding
  controls plus group X/Y offsets. Padding participates in the grid footprint;
  offsets are visual-only and preserve the native OmniButton's semantic order.
- Deliberately did not add cross-tray relocation. Moving the native button to a
  different column would make other mods' `beforeOmni` and `beforeClock` anchors
  disagree with visual order unless all participants share an owned placement
  lease/contract.

## 2026-07-20 — OmniButton native-height default and item-order syntax

- The restoration/centering candidate survived the user's Explorer restart,
  but their near-stock 48px tray screenshot revealed that Smart automatic was
  treating the exact mathematical fit of two 24px rows as usable space. That
  forced a cramped 2×2 layout on standard height. Raised the default automatic
  row pitch to 28px: standard height now selects one row, while taller taskbars
  still admit the balanced 2×2 shape. Explicit `slotHeight` remains an override.
- Reconciled the visible item-order syntax. The YAML default, fallback, README
  presets, and live-test checklist now consistently use comma-separated tokens
  (`wifi, volume, battery, percent`); parsing remains backward compatible with
  spaces and mixed comma/space input.
- Expanded the independent-layout diagnostic to log each child's measured
  width and computed X/Y centering offsets alongside its resolved cell.

## 2026-07-20 — OmniButton compound-percentage root cause

- Follow-up screenshots disproved the one-row default direction: compact 2×2
  on standard height is a core purpose of the mod and the four visuals clearly
  fit. Restored the 24px Smart automatic row pitch, leaving explicit grid modes
  and `slotHeight` as user overrides.
- Root-caused the missing `%`: Windows can expose the battery number and percent
  sign as separate children in the native inner StackPanel. The implementation
  cached, moved, styled, hid, and restored only child 1, stranding child 2 where
  it was clipped. It now owns every child after the battery glyph as one logical
  percentage cluster, measures their combined width, centers the cluster in its
  cell, and applies all percentage controls to every part.
- Added a targeted outer-width lease (`Width`/`MinWidth`/`MaxWidth` only) sized
  to the internal grid footprint. This prevents the native button template from
  clipping a one-row or 2×2 percentage cluster while preserving native height,
  alignment, placement, and exact snapshot restoration on disable.

## 2026-07-20 — OmniButton percentage diagnosis corrected

- Read the live Windhawk diagnostics instead of continuing from the screenshot
  inference. The battery panel has exactly two children: a 20×16 battery Grid
  and one `BatteryTextBlock` whose text is `81%`. There is no separately rendered
  percent-sign child on the tested Windows build; the preceding compound-child
  diagnosis was incorrect.
- Simplified the candidate back to one percentage element while retaining the
  targeted outer-width lease, which addresses clipping independently of the
  mistaken child-count explanation. Changed independent-item centering from
  truncated integer offsets to exact floating-point offsets.
- No source outside OmniButton Customizer was changed by either correction.

## 2026-07-21 — OmniButton production gallery and preflight

- Visually reviewed all six pushed screenshots and added them to both README
  layers with configuration-specific captions. The gallery leads with the new
  standard-height 2×2, reordered row, and busy-tray examples, then documents
  coupled mode and the two tall vertical alternatives. Every image in `assets/`
  is referenced.
- Clarified that group padding is non-negative internal space (0–24px), while
  signed group offsets move the rendered grid (-40–40px). Corrected the
  automatic slot-height documentation from a 20px to the implemented 16px
  minimum. Folder and embedded README content remain synchronized.
- Full production preflight passes: Windhawk compilation, exit-time destructor
  audit, README parity, gallery/link inventory, diff hygiene, symbol-hook policy,
  and the current upstream Windhawk validator. Root catalog now marks v1.0 as a
  submission candidate. Actual submission remains gated on explicit approval
  and a final live disable/restoration/re-enable plus Quick Settings click check.

## 2026-07-21 — OmniButton Customizer v1.0 submitted (PR #3859)

- User confirmed the final source opens Quick Settings and completes clean
  disable/native restoration/re-enable, then explicitly approved submission.
- Maintainer m417z had already approved either replacing or closing the legacy
  Vertical OmniButton PR. Rebuilt its branch from current upstream/main
  `7c214926`, replacing the narrow proposal with one new file:
  `mods/omnibutton-customizer.wh.cpp`.
- Verified the copied source byte-for-byte by SHA-256, reran Windhawk Clang and
  the current upstream validator, and committed it as `b7beff82` (`Add
  OmniButton Customizer v1.0`). Updated PR #3859's title, full feature/test body,
  screenshots, changelog context, and intact authorship section.
- Final GitHub CI is green: changed-file validation and Windhawk 1.6.1, 1.7.3,
  and 2.0.0-alpha.1 compilation all pass. PR:
  https://github.com/ramensoftware/windhawk-mods/pull/3859

## 2026-07-21 — Tray Utility Customizer v2 rewrite (per-icon layout)

Full ground-up rewrite of tray-utility-customizer after live testing proved the
host-moving v1 architecture could not deliver the mod's purpose on build 26200:
Emoji and Touch Keyboard share one Windows host (`NonActivatableStack`), so
moving whole hosts can never stack them vertically, and forcing 24px cells onto
wider native icons broke hover highlights and spacing. Nine iteration rounds in
one session, all under the no-push-without-live-test rule:

- **Two new copy-source templates.** `_templates/nested-group-layout.h` v1.0:
  pure pixel-space layout from one nestable expression (`|` primary axis, `,`
  cross axis, parentheses alternate axes), native-size items, absent tokens
  collapse; unit-tested (`tests/nested-group-layout-tests.cpp` — diamond, both
  axes, spacing, nesting, cross-align, parse failure). `visual-tree-walk.h`
  v1.0: `ForEachDescendant`/`FindDescendant`/`CollectDescendants` plus the
  OmniButton `FindInnerStackPanel`. Both added to `_templates/README.md`.
- **Per-icon control without splitting Windows hosts.** Hosts reparent into one
  owned group (`TrayUtilityCustomizerGroup`); each native `IconView` is steered
  to its target cell with flow-compensating margins (margins participate in
  layout, so flyouts still anchor correctly — the OmniButton "independent mode"
  trick generalized). Chevron and the MainStack fallback are host-leaf items.
  Unplaced visible icons append after the group so nothing is ever lost.
- **Template adoptions carried in:** lifecycle v1.2 (exception-contained
  dispatcher, `TrayUI::StartTaskbar` rehook with stale-tree drop, no_destroy on
  all XAML-owning globals), injected-grid-column v1.2, start-placement bumped to
  **v1.2** (centers the group against the taskbar RootGrid, not Start's padded
  box — fixes vertical mis-centering). smart-grid also gained v1.1 `minColumns`
  + `PackUnits` and v1.2 shortGroupPosition-aware packing during the earlier
  bundle-era rounds; that block was then removed from this mod when the layout
  expression replaced grid modes (template file stays for other adopters).
- **BREAKING settings:** one `layout` expression replaces itemOrder / gridMode /
  smartLayout / gridRows / gridColumns / fillOrder / shortGroup* /
  overflowPlacement. `buttonWidth`/`buttonHeight` default 0 = native size.
  Added `primaryAxis` + `crossAlign`. Deleted the in-mod `enabled` toggle
  (Windhawk's own enable/disable covers it). Tokens accept forgiving aliases
  (chevron/keyboard/pen/touchpad/input), canonicalized from the parsed tree;
  unknown tokens log a warning instead of vanishing.
- **Detection** switched to stable Segoe Fluent glyph codepoints (Emoji U+F353,
  Touch Keyboard U+E765) with accessibility-metadata fallbacks.
- **Dropped as unproven:** a `SetWindowPos` clamp for edge flyouts was written
  then removed — the tray overflow flyout is an explorer-owned windowed popup
  but the fix wasn't confirmed to work, and the Emoji panel lives in
  `TextInputHost.exe` (out of explorer's injection reach). Documented as a known
  limitation instead. Right-of-Start "snaps in only after interaction" got an
  `UpdateLayout()` + immediate re-Position and a 600 ms settle-timer re-Position
  (the centered taskbar re-flows through an animation); still needs live
  confirmation.
- Live-confirmed by the user: v2 default row, chevron-over-row, chevron-leading
  and chevron-trailing stacks, a full single column, a leased tray column, a
  busy double-height tray, and Right-of-Start. Fresh screenshots pushed
  (commit `0089823`, no message) and wired into both README layers
  (README_MATCH). All four template blocks diff-verified verbatim; Windhawk
  clang syntax check and exit-time-destructor audit clean. NOT submitted —
  awaiting a final consolidated live pass and explicit approval.

## 2026-07-22 — Tray Utility v1.1 review fixes submitted

- Read the complete open PR #4841 thread before any external write and
  reconciled m417z's four required review items against the v1.1 rewrite.
  The rewrite had already removed the redundant `enabled` setting and added
  ARM64 support; two items still required local changes.
- Removed the `SystemTray.IconView` constructor hook, its Loaded revokers, the
  SystemTray.dll/Taskbar.View.dll/ExplorerExtensions.dll discovery path, and
  the `LoadLibraryExW` hook. Rebuild handling now uses the existing taskbar.dll
  `TrayUI::StartTaskbar` hook, bounded retry, visibility watchers, and
  layout-intactness check.
- Corrected both README layers: Emoji and touch keyboard have stable glyph
  matching, while pen menu, virtual touchpad, and input indicator currently
  depend on English accessibility text and might not be detected under other
  Windows display languages.
- Full local preflight passed: `COMPILE_OK`,
  `EXIT_TIME_DESTRUCTOR_AUDIT_OK`, `README_MATCH`, and the upstream validator's
  `SUBMISSION_PREFLIGHT_OK`. The user then live-confirmed icon churn,
  Explorer/taskbar rebuild, and disable/native restore/re-enable and explicitly
  approved submission.
- Pushed the lab source/readme as `df1f455`, rebased PR #4841's branch onto
  current upstream/main, copied the source byte-for-byte (SHA-256
  `177693615EA0EDDD101A583174EEA5ED02A7C20F4D0B4003954E2F259899B62C`), and
  pushed it as `43d1ac77`. Rewrote the PR body for the v1.1 feature set and
  current gallery and replied to the maintainer review. GitHub changed-file
  validation plus Windhawk 1.6.1, 1.7.3, and 2.0.0-alpha.1 compilation all
  pass.

## 2026-07-22 — Tray Utility language-neutral detection candidate

- Inspected the installed Windows `SystemTray.dll` rather than constructing a
  translated-keyword table. Windows exposes stable runtime identities for the
  relevant controls: EmojiAndMore, TouchKeyboard, InkWorkspace,
  VirtualTouchpad, Language, and Ime system-tray data-model classes, plus
  language-neutral XAML control/content names.
- Replaced all accessibility-label substring matching with a generic subtree
  identity matcher over runtime data-context class, XAML class/name,
  AutomationId, and the established Emoji/Touch Keyboard glyph codepoints.
  Input detection also recognizes `LanguageTextIconContent` and
  `LanguageImageIconContent`; pen recognizes InkWorkspace identities.
- Updated both README layers to describe language-neutral detection. Full
  preflight passes (`COMPILE_OK`, exit-time-destructor audit, `README_MATCH`,
  upstream validator). Local only and uncommitted; requires a live test of all
  available utilities before PR #4841 is changed again.

## 2026-07-22 — Full open-PR audit

- Verified all six lab-authored upstream PRs live on GitHub. #4443, #4485,
  #4841, #4843, #4844, and #4855 are open, mergeable, `CLEAN`, and green across
  changed-file validation plus Windhawk 1.6.1/1.7.3/2.0.0-alpha.1 builds.
- OmniButton #4855 and Privacy Anchor #4843 have no maintainer comments or
  reviews. Clock Spacer #4443 is legitimately awaiting merge: m417z explicitly
  accepted the standalone route, and the superseded integration PR #68 in
  `m417z/my-windhawk-mods` is already closed.
- Found three PRs needing follow-up. Folder Menus #4485 has a July 21 required
  review covering optional/reset no-destroy containers, removal of the
  IconView/multi-module startup path, single-shot after-init, and retry-handle
  races; its pushed source still contains every flagged pattern. Tray Utility
  #4841 has a second July 22 review requiring a no-destroy ownership correction
  and flagging a likely MainStack phantom-straggler layout gap; both the pushed
  source and local detection candidate still contain the relevant patterns.
- VD Switcher #4844's required `g_settings` correction is already present in
  pushed commit `c5995bd3`, but no response was posted to the maintainer's
  question about who introduced the annotation. Its remaining no-destroy
  containers should be reconciled with the newer optional/reset guidance on a
  future tested update.
- No PR, branch, issue, comment, or source was changed during the audit. Only
  lab notes were reconciled with the live GitHub state.

## 2026-07-22 — Lifecycle v1.3 no-destroy ownership gate

- Corrected the shared lifecycle template after maintainer feedback exposed
  that v1.2's examples encouraged both over-annotation (`g_settings`) and bare
  no-destroy containers whose capacity survives normal unload. V1.3 defines
  three ownership shapes: ordinary destruction for heap-only settings/leases,
  direct no-destroy nullable XAML handles, and no-destroy
  `optional<container>` owners revoked and reset on controlled UI-thread unload.
- Hardened the destructor audit beyond Clang's exit-time warning. It now fails
  bare no-destroy standard containers, no-destroy settings, the known heap-only
  injected-column lease, and optional containers with no `reset()` call.
- Corrected the lifecycle retry model: after-init performs one immediate apply;
  bounded retry belongs to `TrayUI::StartTaskbar`; retry handles are detached
  under an SRW lock and waited/closed outside it, preventing double-close races
  without deadlocking a worker synchronously dispatching to the taskbar thread.
- The v1.3 template compiles with Windhawk's bundled Clang. The strengthened
  gate catches the expected Folder Menus and Tray Utility declarations and
  revealed inherited violations in all six active visual mods, establishing a
  deliberate one-mod-at-a-time rollout rather than another reactive PR fix.

## 2026-07-22 — Folder Menus and Tray Utility local review candidates

- Applied lifecycle v1.3 to Folder Menus: optional-backed event-state storage
  with controlled UI-thread reset, one immediate after-init apply, a locked
  retry-handle handoff, and no IconView/SystemTray multi-module startup path.
- Applied the same ownership/retry model to Tray Utility and fixed the concrete
  MainStack phantom-straggler gap by excluding every whole-host layout item
  from per-icon straggler discovery. Also added forced settings retries, stale
  token revocation, atomic layout state, strict malformed-layout rejection,
  and the requested small metadata/library cleanups.
- Preserved Tray Utility's uncommitted language-neutral detection rewrite.
  Upgraded the shared nested-layout parser/tests to reject unbalanced or empty
  expressions, and hardened the lifecycle template/destructor audit so these
  ownership mistakes are caught before future submissions.
- Both mod candidates pass Windhawk compilation, the strengthened exit-time
  destructor audit, README parity, diff hygiene, and the current upstream
  validator (`SUBMISSION_PREFLIGHT_OK`). No PR, branch, commit, or GitHub
  comment was changed; both candidates await fresh live testing.
- The first Folder Menus live build exposed a hole in that wording: the old
  local compiler gate was syntax-only and therefore missed the accidentally
  removed `-loleaut32`, producing unresolved `SysFreeString`/`SysStringLen`
  symbols. Restored the library and upgraded `compile-check.ps1` to perform a
  real temporary-DLL link using each mod's declared `@compilerOptions`.
  Folder Menus then passed both the new link gate and full submission preflight.
  The same mistaken library removal was restored in Tray Utility immediately
  afterward; it also passes the real link gate and full submission preflight.

## 2026-07-23 — Folder Menus and Tray Utility live confirmation

- The user confirmed that both corrected local builds appear to be working.
  This satisfies the human live-test gate for the current Folder Menus and Tray
  Utility maintainer-review candidates.
- No PR, branch, commit, or GitHub comment was changed. Both tested candidates
  remain local pending explicit approval to update their respective PRs.

## 2026-07-23 — Repo hygiene audit and fork cleanup

- Audited every repo, branch, and fork after an interrupted agent session.
  Nothing upstream was damaged: all six PRs are open/mergeable/`CLEAN`, #4485 is
  still `46f33014` and #4841 still `43d1ac77`. No rogue forks exist; the only
  Windhawk forks are `sb4ssman/windhawk-mods` and `m417z-windhawk-mods`. The
  review candidates are committed to lab `main` as `018a3bd`.
- Found and removed a 27 MB `.tmp-windhawk-mods/` scratch clone of
  ramensoftware/windhawk-mods living untracked and un-ignored inside the lab
  (user deleted it). PR #4485 had actually been pushed from that clone, which is
  why the real fork's `sb4ssman-taskbar-folder-menus` sat at v0.6 `5f7bfe86` —
  a diverged line, not an ancestor of the PR head. Reset it to
  `origin/sb4ssman-taskbar-folder-menus` (`46f33014`); the old tip is kept as
  tag `backup/folder-menus-local-pre-reset`.
- Repaired the fork's `remote.origin.fetch`, which had been narrowed to a single
  explicit refspec for the deleted `sb4ssman-vertical-omnibutton` branch. Plain
  `git fetch origin` had been failing with `fatal: couldn't find remote ref`,
  leaving origin refs stale and divergence invisible. Restored the wildcard and
  set upstream tracking on all ten fork branches.
- Verified all six open-PR branches are clean: each is a one-file diff against
  `upstream/main`, none descends from the fork's `main`, and none carries the
  stray `vertical-omnibutton.wh.cpp`.
- Identified the root cause of the recurring stray-file problem: the fork's
  `main` is ahead of `upstream/main` by ten commits that self-merge
  `mods/vertical-omnibutton.wh.cpp` (fork PR #1) plus merge noise, so anything
  branched from it inherits the file. Recorded as a standing directive; the
  force-push reset of fork `main` awaits user approval.
- Added gitignore guards for nested sister-repo clones and two standing
  directives (one fork checkout only; never branch from the fork's `main`).

## 2026-07-23 — Folder Menus and Tray Utility PR updates pushed

- With explicit user approval after the live test, pushed both review-fix
  candidates: PR #4485 commit `18fe50eb` and PR #4841 commit `20aaf8e5`. Each is
  a single commit on top of the existing branch, still a one-file diff against
  current `upstream/main`, and the pushed blobs are hash-identical to the lab
  candidates. All four CI jobs (changed-file validation, Windhawk 1.6.1, 1.7.3,
  2.0.0-alpha.1) pass on both. Review replies posted.
- Re-ran `submission-preflight.ps1` on both before pushing:
  `COMPILE_OK`, `EXIT_TIME_DESTRUCTOR_AUDIT_OK`, `README_MATCH`,
  `SUBMISSION_PREFLIGHT_OK`. Verified every review item in the source rather
  than trusting the notes.
- Versions deliberately unchanged at 0.7 and 1.1. Neither mod exists in
  upstream `main`, so these are still the proposed initial versions and the
  validator has no prior version to compare against.
- CORRECTION TO THE LIBRARY FINDING: neither mod calls `SysFreeString` or
  `SysStringLen` directly — earlier greps matched `substr` case-insensitively.
  `-loleaut32` is required *transitively*, because `winrt::hresult_error`'s
  constructor and `message()` reference both symbols. Proved by dropping the
  flag and observing `ld.lld: error: undefined symbol: SysStringLen`. Any mod
  that catches `winrt::hresult_error` needs `-loleaut32`; a syntax-only check
  cannot see this, which is why the real link gate exists. `-lversion` was
  genuinely unused and is dropped.

## 2026-07-23 — Fork branch cleanup and PR #4844 reply

- Deleted three dead fork branches, local and remote, after confirming none had
  an open PR: `sb4ssman-taskbar-vd-switcher` (#3932 merged),
  `update-taskbar-vd-switcher` (#4516 merged), and
  `sb4ssman-virtual-desktop-switcher` (#4484 closed). Local `backup/deleted-*`
  tags keep every tip recoverable. The fork now carries exactly the six live PR
  branches plus `main`.
- Answered the maintainer's question on PR #4844: Claude introduced the
  `[[clang::no_destroy]]` annotation, applying it to every namespace-scope
  global with a non-trivial destructor after an earlier exit-time review rather
  than only to WinRT-holding globals. Confirmed the required `g_settings` fix is
  already in `c5995bd3`, and committed publicly to converting the three
  remaining bare containers on the next live-tested update instead of pushing
  untested changes onto a green PR.

## 2026-07-23 — Fork `main` reset (vertical-omnibutton landmine defused)

- Root cause: `sb4ssman/windhawk-mods` `main` had been used as a workspace, so
  it sat ten commits ahead of `upstream/main` — nine merge commits plus
  `94fa7ab6 Add Vertical Omni Button Mod` self-merged via fork PR #1. Content
  difference was a single stray `mods/vertical-omnibutton.wh.cpp` (1409 lines).
  Any branch cut from that main inherited the file, which is what produced the
  two prior "Remove stray vertical-omnibutton.wh.cpp" cleanup commits.
- Fix, with explicit user approval: fetched `upstream`, tagged the old tip
  `backup/fork-main-pre-reset` (`de5feb0f`), `git branch -f main upstream/main`,
  then `git push origin main --force-with-lease`. `origin/main` is now
  hash-identical to `upstream/main` (`9f9f096a`) and the stray file is gone.
- Verified nothing else moved: all six PR branches keep their SHAs and remain
  clean one-file diffs against `upstream/main`. No open PR descended from fork
  `main`, and PR diffs compare against ramensoftware's base, so none changed.
- The dropped mod still exists in the lab at
  `omnibutton-customizer/archive/vertical-omnibutton*.wh.cpp` and was superseded
  upstream by `omnibutton-customizer` (PR #4855); nothing unique was lost.

## 2026-07-23 — Unified element-placement template (nested-group v1.2)

- Decided (with the user) that `nested-group-layout.h` is THE element-placement
  primitive and `smart-grid-layout.h` is demoted to a shape heuristic that emits
  an expression rather than a second arranger. This is element placement only;
  semantic placement ON the taskbar remains a separate, unrelated discussion.
- Extended `nested-group-layout.h` to v1.2:
  * Four-side outer `Padding{left,top,right,bottom}`, each side independently
    addressable, applied once around the whole arranged group; totalSize now
    includes padding.
  * First-class per-element nudge via an optional `OffsetResolver` — a cosmetic
    leaf offset that never moves a neighbor or resizes the group. Old two-arg
    `Compute` overload retained for source compatibility.
  * `BuildGridExpression(count, rows, columns, primaryAxis, rowMajor, namer)` —
    the bridge that turns a smart-grid rows×columns choice into a valid
    expression (row/column-major fill, ragged grids drop empty cells, axis
    transpose). This is the "Auto layout" path; "Manual layout" is a
    user-authored string. Both end at the same `Compute`, so centering, nudge,
    padding, and collapse are identical either way — the basis for a single
    "Layout: Auto / Manual" settings toggle.
- Extended `tests/nested-group-layout-tests.cpp` with padding, nudge,
  padding+nudge composition, and the generator (single row, both fill orders,
  ragged, vertical transpose, round-trip). Both this suite and the existing
  smart-grid suite compile static and pass (exit 0) under Windhawk's bundled
  clang. Documented the unified model in `_templates/README.md`.
- Templates are copy-source, not a live dependency, so existing mods are
  unchanged until they re-adopt; tray-utility still links clean. Next: adopt the
  unified system in VD Switcher (the user-chosen guinea pig) and hand off a
  build for a live test before anything is pushed.

## 2026-07-24 — Two merges, review wave, and the no_destroy resolution

- PR #4485 (Folder Menus v0.7) and PR #4841 (Tray Utility Customizer v1.1) were
  MERGED upstream on 2026-07-23 after the review-fix pushes. Both are now in the
  official catalog.
- The four remaining open PRs each got a fresh 2026-07-23 maintainer review.
  Cross-cutting requests: fix `[[clang::no_destroy]]` usage (all four), a
  physical-px-vs-DIP mixing bug in the smart-grid row heuristic (#4855 blocking,
  #4843), and `SYMBOL_HOOK` array naming that must list every target module
  (#4843, #4855). Per-mod bugs: #4843 has a use-after-free (`wstring_view` over a
  destroyed `hstring`) and a camera-touching default; #4855 has the blocking DPI
  clip; #4443 wants a hooking-surface reduction, a symbol-cache comment fix, and
  a screenshot.
- User replied on #4844 (2026-07-24): "I am updating it, we can hold off on the
  merge... working [the other comments] too." VD Switcher's no_destroy fix is
  already in the unified-placement rebuild (`7c1e4b8`), pending its live test.
- Reconciled working_notes to the merged/open split and the review action items.
- Started Step 3: curate the definitive `[[clang::no_destroy]]` resolution into
  the lifecycle template, then distribute it to #4443/#4843/#4855 one
  live-tested build at a time (VD Switcher already done).

## 2026-07-24 — Settings contract v2, layout rewrite, VD Switcher v2.0

- User rejected the "hybrid" Auto/Manual layout surface outright: the smart part
  was not smart, the granular control was miserable, and the keyed nudge strings
  had to be kept in sync with the layout expression. Thrown out and redesigned
  from the bigger picture — templates and settings first.
- Verified two platform facts that drove the design: nested settings groups work
  and are review-clean (`taskbar-elastic-pill`, read as `Group.Key`), and the
  Windhawk settings API is read-only, so no mod can write a computed value into
  its own field. That killed "smart auto-fills the manual box" and produced the
  single `Layout.Arrangement` field with an `auto` default plus a logged
  expansion.
- Checked all eight PR threads for maintainer pushback on settings backward
  compatibility: none exists. The no-alias rule came from our own reverted v1.6
  `AliasedStr`/`AliasedInt` experiment. Clean break confirmed with the user.
- `settings-profiles.md` rewritten as the settings contract (commit `60204b3`);
  `nested-group-layout.h` rewritten to v2.0; `smart-grid-layout.h` marked
  SUPERSEDED; `verify-settings-order.ps1` added; layout tests rewritten and
  passing (build with `-static`, or the test exe cannot find Windhawk's `.whl`
  runtime).
- Fixed a real gap in the lab tooling: `compile-check.ps1` and
  `exit-time-destructor-audit.ps1` did not pass Windhawk's own Windows version
  defines, so `GetDpiForWindow` failed to compile locally while building fine in
  Windhawk.
- VD Switcher rebuilt as v2.0 on the contract. All five gates green. Not pushed:
  awaiting the live test, per the standing rule.

## 2026-07-24 (cont.) — Layout template v2.0 → v2.4, driven by four live tests

VD Switcher was the guinea pig for the settings contract, and four rounds of
live testing on real hardware drove the template from v2.0 to v2.4. Each defect
was found by running it, not by reading it, and each was fixed in
`_templates/nested-group-layout.h` rather than in the mod:

- **v2.1** — group offsets `(1, 2)[3,0]`, parse errors that report position and
  expectation, token vocabulary rules (identity, never the displayed label).
  Juxtaposition deliberately left an error: a missing separator must not
  silently become a different layout.
- **v2.2** — axis-relative sizing `AlongAxis(thickness, span)`, because an item
  that must match its neighbours cannot take a fixed width and height when a
  hand-written arrangement decides which way it lands. Exposed and fixed a
  latent bug: the grammar's per-unit wrapper was flattening axis-relative sizes
  before the real parent saw them. Also `MissingTokens`/`AppendMissing` so a
  newly created item is never silently unreachable.
- **v2.3** — `TokenMatcher`. `MissingTokens` compared names as strings, so
  `desktop1` looked missing beside expected `1` and got appended again: seven
  buttons instead of four, using the alias the mod itself documents.
- **v2.4** — `RowsInHeight` split from `AvailableRows`, "reserve before you
  divide". The grid was given the whole taskbar height with nothing subtracted
  for a sliver row or outer padding, so the assembled group overflowed (102
  against 96 available). v1.8 had reserved the sliver; the rewrite dropped it.

Mod-side in the same rounds: Task View placement now applies to hand-written
arrangements (it was only ever appended, and appended inside the desktop block
where it squashed), a fifth "last button in the grid" placement, a Task View
gap emitted as a cosmetic offset so a sliver can hang past the taskbar edge, a
`taskview` alias, and the stoplight emoji restored inline. Two stale-value bugs
fell out of the renames: `labelFormat == L"dot"` after the option became
`symbol` (Indicator symbols silently fell back to numbers), and a v1.8 version
string in `Wh_ModInit`.

Tooling: `compile-check.ps1` and `exit-time-destructor-audit.ps1` now pass
Windhawk's own `WINVER/_WIN32_WINNT/NTDDI_VERSION` defines — without them
`GetDpiForWindow` failed locally while building fine in Windhawk.

VD Switcher v2.0 ends the session green on all five gates and DELIBERATELY
UNSHIPPED: the user reversed their own "ship it" so the rest of the family can
be migrated first, in case a later mod forces another template change. See
memory `feedback_template_rollout_before_ship`.

## 2026-07-26 — OmniButton v2.0: three defects, one shared root

The rollout build was live-tested repeatedly, and every defect found came back
to the same mistake in different clothing: **a probe advertised a capability it
had never verified.**

- **The percentage clip.** Three distinct causes, only the third present in
  every run: a glyph-sized cell reserved for text, a cell measured before the
  element was in its final parent, and — the one that actually mattered — the
  natural origin measured against the battery's inner panel while the cell
  positions were in the items host's coordinates, silently adding that panel's
  4px inset to every item. Fixed by measuring in the same space the arrangement
  uses. Verified exactly: battery right edge intended 28, rendered 28.0;
  percent intended 62.6, rendered 62.6.
- **"Why is the OmniButton's area so enormous?"** Not padding, as reported —
  `Size.ItemWidth` defaulted to a 32px box around a ~16px glyph, 16px of dead
  space per item, and `ItemSpacing` was clamped at 0 so it could not be pulled
  back. Fixed with fit-to-content sizing (`ItemWidth: 0`, now the default) and
  negative spacing. `Adjust.PadX` is outer padding and never could have helped;
  both READMEs now say so outright.
- **Wifi and volume refused colour.** `InnerTextBlock` inside a
  `SystemTray.IconView` is TEMPLATE-BOUND — a local write is re-asserted away.
  Fixed with `FindStyleAnchor`, which walks UP to the outermost Control between
  leaf and host and writes there first. Battery and percentage had always
  worked because they are not inside an IconView; that asymmetry was the tell.
- **The ghost.** With colour working, a glyph size produced a large glyph
  hovering over the original. Wifi and volume are each drawn by THREE stacked
  `AdaptiveTextBlock`s (Underlay / Base / AccentOverlay) and the battery by two;
  resizing one layer pulls it off the others. `native-glyph-surface.h` v1.2 now
  COUNTS the glyphs and gates `fontSize`/`fontFamily` on `glyphLayers <= 1`, so
  only the percentage — genuinely one piece of text — offers them.

My own diagnostic bug cost a round: `LogItemSubtree` truncated at depth 4, a
limit tuned for the battery's shallow tree, so the IconView dump ended before
reaching anything useful and answered nothing. maxDepth is now a parameter.

**The OS-setting bridge was removed at the user's direction, and it was the
right call.** `Content.Percent` had driven the Windows battery-percentage
setting. Four registry reads proved the mod overrode the user's own choice at
load; write-on-change fixed that, but Explorer still only sometimes re-read the
value and the Settings page never live-refreshed. A control that works once and
then appears dead is worse than none. The template survives as v1.1 with the
evidence and a "DO NOT RE-ASSERT AT LOAD" doctrine block; no mod embeds it.

Doctrine written into `settings-profiles.md`: **before a probe advertises a
capability, it must check the thing that would make that capability wrong.
"I found one" is not "there is only one."**

## 2026-07-31 — OmniButton live-tested and prepped for PR #4855

The user live-tested v2.0 at 100%, 125% and 150% display scaling and confirmed
it — the first mod in the family to clear the standing no-push-without-a-live-
test rule, and the scaling coverage m417z asked for on the blocking DPI item.

Submission prep found the shape of the job was not what it looked like:
**#4855 is already OPEN**, titled "Add OmniButton Customizer v1.0", branch
`add-omnibutton-customizer`, still carrying `@version 1.0`. So this is an
update to a live PR with review history, not a fresh submission. (Upstream is
`ramensoftware/windhawk-mods`; `m417z/windhawk-mods` does not resolve, which
cost a wrong turn.)

Re-reading the review against the current code rather than trusting the notes
found one item that had survived every previous pass: **the unused `slotW`
parameter m417z explicitly named.** Removed. Both blocking items and five of
the optional ones are confirmed fixed; four optional items are deliberately
declined, stated plainly in the drafted PR body rather than left silent.

The preflight's unreferenced-asset gate earned its keep, catching four 1.x-era
screenshots. One was kept and referenced (`percent-emphasized` — the only shot
of the mod's single remaining font control); three were deleted as advertising
removed features or duplicating the 2×2 hero. `in-a-diamond.png` went into both
READMEs and into the Arrangement examples as
`wifi | (volume, battery) | percent`, the best demonstration of the nesting
grammar the mod has.

Version stays at 2.0 by the user's call, with the READMEs rewritten to explain
it: v1.0 never shipped, so the number marks the settings contract and the
arrangement component, not a release history.

Ends green on all six gates, fully prepped, NOTHING PUSHED.

## 2026-07-31 — OmniButton lab prep pushed; Network identity correction opened

- Committed the reviewed OmniButton v2.0 preparation, README/gallery cleanup,
  roadmap notes, and obsolete screenshot deletions as lab commit `ef32687` and
  pushed it normally to Mod Lab `main`. No force push was used. The fork's
  `add-omnibutton-customizer` branch and upstream PR #4855 were not changed.
- A submission-stop question exposed a naming error before the PR update:
  Windows' first ControlCenterButton item is one Network slot whose glyph
  represents Wi-Fi or Ethernet and also carries disconnected, airplane-mode,
  and VPN states. The implementation handled those glyph changes because it
  operates on the native presenter, but the public `Wifi` setting and `wifi`
  arrangement token were misleading.
- Started a local follow-up that changes the public identity to
  `Network`/`network` and adds the user's new
  `compact-no-percent-stack-nudged.png` gallery image. This settings-contract
  change was subsequently live-confirmed on 2026-08-01 with a written
  four-item arrangement and per-item offsets. The corrected
  `compact-with-nudges.png` example was added to the gallery as well. Both
  README layers and the root catalog were updated; the follow-up remains
  uncommitted at this point.

## 2026-08-03 — OmniButton Customizer v2.0 SUBMITTED (PR #4855)

The first mod in the family to reach a maintainer on the 2.0 settings contract.

- **The blocker was never the code.** The mod had been live-tested at 100/125/150%
  scaling on 2026-08-01 and every gate was green; what stopped it was that the
  gallery had been replaced without being rewired. Nine screenshots were
  referenced and all nine had been deleted; nine new ones sat on disk with none
  referenced. Both README layers and the `.wh.cpp` readme block were repointed.
- **Capability audit against the `@description`, not the settings block.** Five
  of the eight new shots covered arrangement variations while *color* — the
  headline claim in the one-line description that shows on the catalog page —
  had no screenshot at all. The user shot `with-colors.png` to close it.
  Opacity, `PercentFontFamily`, fixed `ItemWidth`, and the `Adjust` offsets were
  deliberately left unphotographed per the gallery policy in `.agents/README.md`.
- **Real arrangement strings replaced inferred ones.** Captions written from
  reading the pictures were plausible and wrong in form — a column-formed `2x2`
  where the user had actually used a row-formed one with nudges. The three
  gallery examples now carry the user's verbatim working strings. Precedence
  (`,` binds tighter than `|`) was verified against the parser in
  `nested-group-layout.h` before being asserted twice in user-facing docs.
- **Submission verified against upstream rather than the notes**, after the user
  challenged the procedure: `pr_validation.py:1164` requires
  `(added, modified, all)` in `[(1,0,1),(0,1,1)]`, and line 1172 requires the
  literal `## Mod authorship` heading for an added-file PR, read AT PUSH TIME.
  Because `omnibutton-customizer` is not in `upstream/main`, this is the
  one-ADDED-file case.
- Fork branch amended to one clean commit `a6bde2de` and force-pushed with
  lease (local and origin were verified identical first, so the earlier
  `reset --hard` in the plan was dropped as needlessly destructive). Diff vs
  `upstream/main` is exactly one added file, byte-identical to the lab copy.
  PR retitled to v2.0 and the body replaced wholesale — the old one advertised
  removed v1.0 features and embedded deleted screenshots.
- All five CI jobs pass: changed-file validation plus Windhawk 1.6.1, 1.7.3,
  and 2.0.0-alpha.2 compilation.

## 2026-08-03 — Privacy Anchor template rollout, live-confirmed

Took the mod from 3 embedded templates to 8 — the most templated in the
family — and the user live-tested and confirmed it, camera detection included.

- **Adopted:** `color-tokens.h` (retires the third copy of the token parser),
  `visual-tree-walk.h`, `settings-io.h`, `taskbar-host.h`, `property-lease.h`.
  **Re-embedded:** `nested-group-layout.h` v2.5 and `start-placement.h` v1.2.
- **The start-placement drift the notes had recorded as "previously UNKNOWN"
  is identified and fixed.** v1.2 centers the group against the taskbar root's
  height because Start's own box is not a reliable vertical reference; the mod
  still had v1.1's math. Affects `leftOfStart` / `rightOfStart` only.
- **The native-indicator restore no longer guesses.** It used to re-derive what
  an icon's visibility "should" be from whether its glyph had text — a good
  guess, but one that cannot express "there was no local value here", which is
  the normal case for a template-bound tray icon where the right restore is
  `ClearValue`. Every suppression write is now leased and restored exactly.
- Gained the vertical-taskbar stand-down (previously OmniButton-only), moved
  row capacity onto `tbh::GetMetrics`, and made `g_settings` heap-free.
- **Not adopted, deliberately:** `tbh::RetryLoop`, because this mod's retry
  thread is fused with the privacy state worker and shares its stop event; and
  `native-glyph-surface.h`, because the mod draws its own icons and has no
  template-bound native leaf to style.

**A false alarm that produced real doctrine.** The user reported that camera
hardware detection had stopped working and that the rollout had removed it. It
had not: `CameraPrivacyMonitor` was intact and the rollout touched two camera
lines, both semantically identical settings reads. The actual cause was commit
`92e767e` from the earlier 2.0 rework, which renamed
`cameraHardwareDetection` to `Behavior.CameraHardwareDetection` **and** flipped
its default from true to false in the same change. Windhawk cannot carry a
value across a renamed key, so the user's "on" was dropped and the new key read
as off — an entire capability went dark with no symptom.

Two rules came out of it. **Never rename a settings key and change its default
in the same change**; either alone is recoverable, together they silently
revert a user's choice and leave no trace. And **when an opt-in setting gates a
whole capability rather than tuning one, say so at runtime** — the init log now
names which switch is off and what is lost, and the camera's idle tooltip names
the setting on the taskbar itself rather than only in a log nobody has open.
