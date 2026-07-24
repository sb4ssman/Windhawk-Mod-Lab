# Working Notes — Windhawk Mod Lab

Living todo list — current state only, pruned every session. Completed work
(past tense) goes in [work_log.md](work_log.md). Durable rules live in
[README.md](README.md); reference material in [knowledge/](knowledge/).

## Current focus

**MERGED 2026-07-23:** Taskbar Folder Menus (PR #4485) and Tray Utility
Customizer (PR #4841) are now in the official catalog. No further action.

**Active work: cross-cutting `[[clang::no_destroy]]` resolution.** The
2026-07-23 review wave requests the same no_destroy fix on every remaining open
mod (#4843, #4844, #4855) — it was also the merge-gate on the two that merged.
Curate the definitive resolution into the lifecycle template, then distribute to
each affected mod one live-tested build at a time. VD Switcher already has the
fix (in the unified-placement rebuild, `7c1e4b8`, pending its live test).

VD Switcher rebuild on unified nested-group placement is committed at `7c1e4b8`
(compiles + audit-green, preliminary user test "looking OK"); READMEs + version
bump are deferred until the full live test. See work_log 2026-07-23.

## Open upstream PRs — reconciled 2026-07-24

Two merged; four open, each now carrying a 2026-07-23 maintainer review. Never
push a review fix before a fresh live test.

- Taskbar Folder Menus v0.7 — PR #4485: **MERGED** (`18fe50eb`).
- Tray Utility Customizer v1.1 — PR #4841: **MERGED** (`20aaf8e5`).
- Taskbar VD Switcher v1.8 — PR #4844: OPEN. User replied 2026-07-24 "I am
  updating it, we can hold off on the merge... working [the other comments]
  too." The unified-placement rebuild (`7c1e4b8`) already includes the
  no_destroy conversion; awaits full live test, READMEs, version bump before the
  PR is updated.
- Taskbar Clock Spacer v1.1 — PR #4443: OPEN, ACTION REQUIRED. Review asks: (1)
  why hook everything vs just `DateTimeIconContent::OnApplyTemplate`; (2) remove
  `(pre-2604)` from a comment (breaks symbol-cache parser); (3) drop no_destroy
  on `g_states` (weak_ref release is thread-safe; bare container leaks buffer);
  (4) add before/after screenshot; optional off-thread-cleanup + scan-thread
  race notes.
- Privacy Indicator Anchor v1.0 — PR #4843: OPEN, ACTION REQUIRED. Two real
  bugs: (1) use-after-free — `wstring_view` over a destroyed `hstring` temporary
  on the hot scan path (take a `std::wstring` copy); (2) `cameraHardwareDetection`
  defaults true → touches camera on every Explorer start, make it opt-in false.
  Plus: no_destroy fix; rename `systemTrayDllHooks` (3 modules) + comment; DPI
  mixing (physical px ÷ DIP pitch) in row math. Optional: mic-monitor cleanup
  race, StringSetting RAII, Copilot poll cadence.
- OmniButton Customizer v1.0 — PR #4855: OPEN, ACTION REQUIRED. (1) DPI bug
  (BLOCKING) — `ResolveGeometry` mixes physical px and DIPs → grid clips above
  100% scaling; fix with `MulDiv` + `GetDpiForWindow`. (2) rename
  `systemTrayDllHooks` (3 modules) + comment. Plus: no_destroy fix; test at
  varied DPI. Optional: retry-thread race, IsWindow revalidation, dup
  LoadColorSetting, slotWidth<16 clamp, unused params/includes.

### Cross-cutting themes from the 2026-07-23 review wave
- **`[[clang::no_destroy]]`** — requested on all four open mods; merge-gate on
  the two merged. Being resolved via the lifecycle template (this session).
- **DPI mixing (physical px vs DIPs)** — real bug on #4855 (blocking) and #4843;
  lives in the smart-grid `GetAvailableRows` heuristic shared across mods. The
  unified-placement rollout should centralize a DIP-correct height.
- **`SYMBOL_HOOK` array naming** — must name every target module; #4843, #4855.

Follow-ups:

- [ ] Clock Spacer: optional fresh gallery and residual restart/disable/token
      checks. m417z/my-windhawk-mods PR #68 is already closed.
- [ ] VD Switcher: verify issue #4784 hover/hit-test and test `multiMonitor`
      when Explorer restarts will not disrupt the user's desktop order.
- [ ] Privacy Anchor: optional final-build restart/hardware-monitor checks and
      idle screenshot refresh; keep frame-signature inference opt-in only.
- [ ] Tray Utility v1.1 (per-icon rewrite) — submitted in PR #4841.
      Full history in work_log 2026-07-21 "Tray Utility Customizer v2
      rewrite". State: `COMPILE_OK`, exit-time-destructor audit OK,
      `README_MATCH`, and upstream validator `SUBMISSION_PREFLIGHT_OK`.
      Settings/template audit and gallery work are complete. Maintainer item
      1 was initially addressed by documenting the English limitation. A new
      local candidate now removes that limitation entirely: it matches
      Windows' `EmojiAndMore`, `TouchKeyboard`, `InkWorkspace`,
      `VirtualTouchpad`, `Language`, and `Ime` system-tray data-model classes,
      plus exact XAML/Automation identities and established glyphs. No
      localized accessibility words participate. Automated preflight passes;
      live-test all available utilities before another PR update. Item 3 is addressed
      by removing the IconView constructor hook, the SystemTray/Taskbar.View/
      ExplorerExtensions dependency, and the LoadLibraryExW hook; lifecycle
      now relies on taskbar.dll `TrayUI::StartTaskbar`, the bounded initial/
      rebuild retry, visibility watchers, and the layout-intactness check.
      User live-confirmed icon appearance/churn, Explorer/taskbar rebuild, and
      disable→native restore→re-enable. PR commit `43d1ac77`; changed-file
      validation and Windhawk 1.6.1/1.7.3/2.0.0-alpha.1 compile jobs all pass.
      Await maintainer review. Two documented limitations remain (see below).
- [ ] Tray Utility edge flyout clipping — ACCEPTED LIMITATION (both READMEs
      document it). The SetWindowPos work-area clamp was tried TWICE
      (2026-07-21) and REMOVED both times — it never fixed the overflow
      flyout, so `Xaml_WindowedPopupClass` + a window-move hook is the wrong
      lever. Working theory: the overflow flyout is an in-process XAML Popup
      positioned in DIP space off the taskbar XamlRoot (no separate HWND that
      SetWindowPos would touch), so a real fix would need a XAML-layer
      intervention (find the transient Popup, adjust placement/offset) — high
      risk for a cosmetic edge-only issue, deferred. The Emoji panel is drawn
      by `TextInputHost.exe` (different process) — genuinely out of reach from
      explorer injection; would need `@include TextInputHost.exe`. Do NOT
      re-attempt the SetWindowPos clamp. If ever revisited, FIRST capture the
      real overflow-flyout window (Spy++) to confirm whether it's even a
      separate HWND.
- [ ] PRIORITY — Tray Utility Right/Left of Start settle: the centered
      taskbar re-flows through an animation after the hosts leave the tray;
      current mitigation is `UpdateLayout()` + immediate re-Position + a
      600 ms settle-timer re-Position. Needs live confirmation it fully
      fixes the "settles only after interaction" behavior. Always-on [Start]
      log (inRepeater flag, Start rect, group margin, root size) is present
      for diagnosis.
- [ ] Tray Utility FEATURE IDEA (user, 2026-07-21): "Always show these
      icons" switch — drive the Windows taskbar-settings visibility toggles
      for the selected utilities (touch keyboard TipbandDesiredVisibility,
      pen ShowPenWorkspaceButton, touchpad) instead of an in-mod enable
      toggle. Deferred; needs its own investigation.
## Lab-level todos
- [x] FORK `main` RESET — DONE 2026-07-23. `sb4ssman/windhawk-mods` `main` was
      reset onto `upstream/main` and force-pushed; `origin/main` is now
      hash-identical to `upstream/main` (`9f9f096a`) with no
      `vertical-omnibutton.wh.cpp`. Old tip kept as tag
      `backup/fork-main-pre-reset` (`de5feb0f`). All six PR branches verified
      unchanged and still one-file diffs. Landmine defused; keep it that way by
      never branching from fork `main` (see README directive).
- [x] STALE FORK BRANCHES — DONE 2026-07-23. Deleted
      `sb4ssman-taskbar-vd-switcher`, `update-taskbar-vd-switcher`, and
      `sb4ssman-virtual-desktop-switcher` local + remote; `backup/deleted-*`
      tags retained. Fork now carries exactly the six live PR branches + `main`.
- [ ] LIFECYCLE v1.3 ROLLOUT — the corrected template distinguishes heap-only
      state, direct XAML handles, and optional-backed XAML containers with
      controlled UI-thread `reset()`. Folder Menus and Tray Utility now have
      local preflight-green, live-tested adoptions. The strengthened audit
      still intentionally fails VD Switcher, Privacy Anchor, OmniButton, and
      Clock Spacer; roll those out one live-tested mod at a time before their
      next PR updates.
- [ ] FUTURE TEMPLATE EXTRACTION — the tray-utility v2 "positioning system"
      is only PARTLY templated. Done: `nested-group-layout.h` (expression →
      pixel placements, tested). NOT done: the orchestration in
      `ApplyLayout` — reparent native hosts into one owned group + steer each
      IconView to its target via flow-compensating margins + anchor selection
      (borrow column / injected-column lease / start-placement lease). That
      per-icon reparent+margin-steer pattern is the novel reusable piece;
      extract it into e.g. `tray-group-placement.h` (with the OmniButton
      "independent mode" as a second data point) when another mod needs it.
- [ ] TOOLING: UWPSpy is available (XAML-tree inspection) — see memory
      `reference_uwpspy`. Use it (not Spy++) to investigate whether the
      tray-utility overflow flyout is an in-process XAML Popup before any
      further edge-clip attempt.
- [ ] TEMPLATE UNIFORMITY IS A STANDING PRIORITY (user, 2026-07-19): mods keep
      shipping without the curated `_templates/` profiles (settings names,
      order, smart grid). Every mod touched must be checked against
      `_templates/settings-profiles.md`, the applicable copy-source templates,
      and `six-mod-settings-audit.md` before screenshots/PR. OmniButton's
      applicable gaps are resolved in the current test candidate; Tray Utility
      v2 adopted lifecycle v1.2 (now superseded by v1.3), start-placement v1.2,
      injected-column v1.2,
      and the two new layout/tree-walk templates, but its new `layout`/
      `primaryAxis`/`crossAlign` settings still need a `settings-profiles.md`
      audit pass before any PR.
- [ ] windhawk-mods PR update script (pull upstream → copy .wh.cpp →
      create/update PR); fork at `t:/Github/sb4ssman/windhawk-mods/`

## Future ideas (unscheduled)

- Densification layout (see work log 2026-05-08; `_research/densification-analysis.md`)
- Privacy anchor filler/status mode (mic dB meter, globe affordance)
- Folder-menus chevron-area sharing experiment
- Tray Stats Panel mod (free-standing stats panel)
- OmniButton glyph color animations — archived by user direction, needs a
  design conversation first (knowledge/omnibutton-color-animation-archived.md)
